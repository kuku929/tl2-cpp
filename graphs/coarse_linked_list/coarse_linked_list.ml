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
  match Xt.get xt pred.next with
  | None -> (pred, pred)
  | Some curr ->
      if curr.key >= key then
        (pred, curr)
      else
        locate xt curr key

(* add *)
let add list item =
  let key = Hashtbl.hash item in
  Xt.commit (fun xt ->
    let (pred, curr) = locate xt list.head key in
    if curr.key = key then
      false
    else (
      let node =
        { item; key; next = Loc.make (Some curr) }
      in
      Xt.set xt pred.next (Some node);
      true
    )
  )

(* remove *)
let remove list item =
  let key = Hashtbl.hash item in
  Xt.commit (fun xt ->
    let (pred, curr) = locate xt list.head key in
    if curr.key = key then (
      let next = Xt.get xt curr.next in
      Xt.set xt pred.next next;
      true
    ) else
      false
  )

(* contains *)
let contains list item =
  let key = Hashtbl.hash item in
  Xt.commit (fun xt ->
    let rec loop curr =
      if curr.key >= key then
        curr.key = key
      else
        match Xt.get xt curr.next with
        | None -> false
        | Some next -> loop next
    in
    loop list.head
  )

(* Basic test *)
let () =
  let l = create () in

  assert (add l 10);
  assert (contains l 10);
  assert (not (add l 10));
  assert (remove l 10);
  assert (not (contains l 10));

  Printf.printf "Basic test passed\n"