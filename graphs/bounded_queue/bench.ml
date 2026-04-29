open Kcas

type 'a t = {
  capacity : int;
  items : 'a Loc.t array;
  head : int Loc.t;
  tail : int Loc.t;
}

let create cap init =
  {
    capacity = cap;
    items = Array.init cap (fun _ -> Loc.make init);
    head = Loc.make 0;
    tail = Loc.make 0;
  }

let try_enq q x =
  let success = ref false in
  Xt.commit {
    tx = (fun ~xt ->
      let h = Xt.get ~xt q.head in
      let t = Xt.get ~xt q.tail in

      if t - h = q.capacity then (
        success := false
      ) else (
        Xt.set ~xt q.items.(t mod q.capacity) x;
        Xt.set ~xt q.tail (t + 1);
        success := true
      )
    )
  };
  !success

let try_deq q =
  let result = ref None in
  Xt.commit {
    tx = (fun ~xt ->
      let h = Xt.get ~xt q.head in
      let t = Xt.get ~xt q.tail in

      if t = h then (
        result := None
      ) else (
        let v = Xt.get ~xt q.items.(h mod q.capacity) in
        Xt.set ~xt q.head (h + 1);
        result := Some v
      )
    )
  };
  !result

(* -------- benchmark -------- *)

let benchmark_batch domains ops =
  let q = create 1024 0 in

  let start = Unix.gettimeofday () in

  let worker () =
    for i = 1 to ops do
      Xt.commit {
        tx = (fun ~xt ->
          ignore(xt);
          for k = 1 to 10 do
            ignore (try_enq q (i + k));
            ignore (try_deq q)
          done
        )
      }
    done
  in

  let ds = Array.init domains (fun _ -> Domain.spawn worker) in
  Array.iter Domain.join ds;

  let stop = Unix.gettimeofday () in

  let throughput =
    float_of_int (domains * ops) /. (stop -. start)
  in

  Printf.printf "%d,%f\n%!" domains throughput

(* -------- main -------- *)

let () =
  let ops = 100_000 in
  let thread_counts = [1; 2; 4; 8;] in

  (* CSV header *)
  Printf.printf "threads,throughput\n%!";

  List.iter (fun d ->
    benchmark_batch d ops
  ) thread_counts