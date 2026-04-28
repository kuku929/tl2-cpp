open Kcas

type 'a queue = {
  cap : int;
  items : 'a Loc.t array;
  head : int Loc.t;
  tail : int Loc.t;
}

let create cap default =
  {
    cap;
    items = Array.init cap (fun _ -> Loc.make default);
    head = Loc.make 0;
    tail = Loc.make 0;
  }

let try_enq q x =
  Xt.commit {
    tx = (fun ~xt ->
      let h = Xt.get ~xt q.head in
      let t = Xt.get ~xt q.tail in
      if t - h = q.cap then false
      else (
        Xt.set ~xt q.items.(t mod q.cap) x;
        Xt.set ~xt q.tail (t + 1);
        true))
  }

let try_deq q =
  Xt.commit {
    tx = (fun ~xt ->
      let h = Xt.get ~xt q.head in
      let t = Xt.get ~xt q.tail in
      if t = h then None
      else (
        let v = Xt.get ~xt q.items.(h mod q.cap) in
        Xt.set ~xt q.head (h + 1);
        Some v))
  }

let print_header name =
  Printf.printf "\n# %s\nthreads,throughput\n%!" name

let benchmark name f =
  print_header name;
  List.iter (fun domains -> f domains 100000) [1;2;4;8]

let run domains ops worker =
  let start = Unix.gettimeofday () in
  let ds = Array.init domains (fun _ -> Domain.spawn worker) in
  Array.iter Domain.join ds;
  let stop = Unix.gettimeofday () in
  float_of_int (domains * ops) /. (stop -. start)

(* -------- LOW CONTENTION -------- *)
let low_contention domains ops =
  let qs = Array.init domains (fun _ -> create 1024 0) in
  let worker id () =
    let q = qs.(id) in
    for i = 1 to ops do
      if i mod 2 = 0 then ignore (try_enq q i)
      else ignore (try_deq q)
    done
  in
  let throughput =
    let start = Unix.gettimeofday () in
    let ds =
      Array.init domains (fun i -> Domain.spawn (worker i))
    in
    Array.iter Domain.join ds;
    let stop = Unix.gettimeofday () in
    float_of_int (domains * ops) /. (stop -. start)
  in
  Printf.printf "%d,%f\n%!" domains throughput

(* -------- READ HEAVY -------- *)
let read_heavy domains ops =
  let q = create 1024 0 in
  let worker () =
    for i = 1 to ops do
      if i mod 10 = 0 then ignore (try_enq q i)
      else ignore (try_deq q)
    done
  in
  let throughput = run domains ops worker in
  Printf.printf "%d,%f\n%!" domains throughput

(* -------- BATCH -------- *)
let batch domains ops =
  let q = create 1024 0 in
  let worker () =
    for i = 1 to ops do
      Xt.commit {
        tx = (fun ~xt ->
          ignore(xt);
          for k = 1 to 10 do
            ignore (try_enq q (i+k));
            ignore (try_deq q)
          done)
      }
    done
  in
  let throughput = run domains ops worker in
  Printf.printf "%d,%f\n%!" domains throughput

let () =
  benchmark "low_contention" low_contention;
  benchmark "read_heavy" read_heavy;
  benchmark "batch" batch