open Printf

module Stack = Kcas_data.Stack

(* ---------------- Benchmark ---------------- *)

let benchmark threads ops =
  let st = Stack.create () in

  let start = Unix.gettimeofday () in

  let domains =
    Array.init threads (fun t ->
      Domain.spawn (fun () ->
        for _ = 1 to ops do
          if (t land 1) = 1 then begin
            Stack.push 2 st;
            Stack.push 4 st;
          end else begin
            ignore (Stack.pop_opt st);
            ignore (Stack.pop_opt st);
          end
        done
      )
    )
  in

  Array.iter Domain.join domains;

  let stop = Unix.gettimeofday () in
  let sec = stop -. start in

  (* throughput = total ops / time *)
  let total_ops = threads * ops * 2 in
  printf "%d,%f\n%!" threads (float total_ops /. sec)

(* ---------------- Main ---------------- *)

let () =
  let ops = 100_000 in
  let thread_counts = [1;2;4;6;8;10;12;14;16;18;20;24;30] in

  printf "threads,throughput\n";

  List.iter (fun t -> benchmark t ops) thread_counts