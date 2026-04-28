let benchmark_batch domains ops =
  let q = create 1024 0 in

  let start = Unix.gettimeofday () in

  let worker () =
    for i = 1 to ops do
      Xt.commit {
        tx = (fun ~xt ->
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