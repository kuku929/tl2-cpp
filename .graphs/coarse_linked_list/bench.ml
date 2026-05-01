open Kcas

(* Node definition *)
type 'a node = {
  item : 'a;
  key : int;
  next : 'a node option Loc.t;
}

(* List structure *)
type 'a t = {
  head : 'a node;
  tail : 'a node;
}

(* Create list with sentinels *)
let create () =
  let tail =
    { item = Obj.magic 0; key = max_int; next = Loc.make None }
  in
  let head =
    { item = Obj.magic 0; key = min_int; next = Loc.make (Some tail) }
  in
  { head; tail }

(* Locate: must be called inside transaction *)
let rec locate xt pred key =
  match Xt.get ~xt pred.next with
  | None -> (pred, pred)
  | Some curr ->
      if curr.key >= key then
        (pred, curr)
      else
        locate xt curr key
(*
(* add *)
let add list item =
  let key = Hashtbl.hash item in
  Xt.commit {
    tx = (fun ~xt ->
      let (pred, curr) = locate xt list.head key in
      if curr.key = key then
        false
      else (
        let node =
          { item; key; next = Loc.make (Some curr) }
        in
        Xt.set ~xt pred.next (Some node);
        true
      )
    )
  }

(* remove *)
let remove list item =
  let key = Hashtbl.hash item in
  Xt.commit {
    tx = (fun ~xt ->
      let (pred, curr) = locate xt list.head key in
      if curr.key = key then (
        let next = Xt.get ~xt curr.next in
        Xt.set ~xt pred.next next;
        true
      ) else
        false
    )
  }

(* contains *)
let contains list item =
  let key = Hashtbl.hash item in
  Xt.commit {
    tx = (fun ~xt ->
      let rec loop curr =
        if curr.key >= key then
          curr.key = key
        else
          match Xt.get ~xt curr.next with
          | None -> false
          | Some next -> loop next
      in
      loop list.head
    )
  }
*)
let add_tx xt list item =
  let key = Hashtbl.hash item in
  let (pred, curr) = locate xt list.head key in
  if curr.key = key then
    false
  else (
    let node = { item; key; next = Loc.make (Some curr) } in
    Xt.set ~xt pred.next (Some node);
    true
  )

let remove_tx xt list item =
  let key = Hashtbl.hash item in
  let (pred, curr) = locate xt list.head key in
  if curr.key = key then (
    let next = Xt.get ~xt curr.next in
    Xt.set ~xt pred.next next;
    true
  ) else
    false

let benchmark_batch domains ops =
  let l = create () in

  let start = Unix.gettimeofday () in

  let worker () =
    for i = 1 to ops do
      Xt.commit {
        tx = (fun ~xt ->
          for k = 1 to 10 do
            ignore (add_tx xt l (i + k));
            ignore (remove_tx xt l (i + k));
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

let () =
  let ops = 100_000 in
  let thread_counts = [1; 2; 4; 8] in

  Printf.printf "threads,throughput\n%!";

  List.iter (fun d ->
    benchmark_batch d ops
  ) thread_counts