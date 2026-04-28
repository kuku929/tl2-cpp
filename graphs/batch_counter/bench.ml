open Kcas

let benchmark domains iters =
  let counter = Loc.make 0 in

  let start = Unix.gettimeofday () in

  let worker () =
    for _ = 1 to iters do
      Xt.commit {
        tx = (fun ~xt ->
          let v = Xt.get ~xt counter in
          let v' = ref v in
          for _ = 1 to 1000 do
            v' := !v' + 1
          done;
          Xt.set ~xt counter !v'
        )
      }
    done
  in

  let ds = Array.init domains (fun _ -> Domain.spawn worker) in
  Array.iter Domain.join ds;

  let stop = Unix.gettimeofday () in
  let throughput =
    float_of_int (domains * iters) /. (stop -. start)
  in

  Printf.printf "%d,%f\n%!" domains throughput

let () =
  Printf.printf "threads,throughput\n";
  List.iter (fun d -> benchmark d 100000) [1;2;4;8]