open Kcas
open Kcas_data

let benchmark domains ops_per_domain =
  let q = Queue.create () in

  let start = Unix.gettimeofday () in

  let worker () =
    for i = 1 to ops_per_domain do
      if i mod 2 = 0 then
        Queue.add i q
      else
        ignore (Queue.take_opt q)
    done
  in

  let ds =
    Array.init domains (fun _ -> Domain.spawn worker)
  in

  Array.iter Domain.join ds;

  let stop = Unix.gettimeofday () in
  let elapsed = stop -. start in

  let total_ops = float_of_int (domains * ops_per_domain) in
  let throughput = total_ops /. elapsed in

  Printf.printf "%d,%f\n%!" domains throughput

let () =
  Printf.printf "domains,throughput\n";
  List.iter (fun d -> benchmark d 100000) [1;2;4;8]