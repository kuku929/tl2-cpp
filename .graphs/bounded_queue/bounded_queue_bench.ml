open Bounded_queue  (* your file name must match module *)

let benchmark domains ops_per_domain =
  let q = create 1024 0 in

  let start = Unix.gettimeofday () in

  let worker () =
    let dummy = ref 0 in
    for i = 1 to ops_per_domain do
      if i mod 2 = 0 then
        ignore (try_enq q i)
      else
        match try_deq q with
        | Some v -> dummy := !dummy + v
        | None -> ()
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
  Printf.printf "threads,throughput\n";
  List.iter (fun d -> benchmark d 100000) [1;2;4;8]