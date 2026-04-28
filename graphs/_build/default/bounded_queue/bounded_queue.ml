open Kcas

type 'a bounded_queue = {
  capacity : int;
  items : 'a Loc.t array;
  head : int Loc.t;
  tail : int Loc.t;
}

let create capacity default =
  {
    capacity;
    items = Array.init capacity (fun _ -> Loc.make default);
    head = Loc.make 0;
    tail = Loc.make 0;
  }

let try_enq q x =
  Xt.commit {
    tx = (fun ~xt ->
      let h = Xt.get ~xt q.head in
      let t = Xt.get ~xt q.tail in

      if t - h = q.capacity then
        false
      else (
        let idx = t mod q.capacity in
        Xt.set ~xt q.items.(idx) x;
        Xt.set ~xt q.tail (t + 1);
        true
      )
    )
  }

let try_deq q =
  Xt.commit {
    tx = (fun ~xt ->
      let h = Xt.get ~xt q.head in
      let t = Xt.get ~xt q.tail in

      if t = h then
        None
      else (
        let idx = h mod q.capacity in
        let v = Xt.get ~xt q.items.(idx) in
        Xt.set ~xt q.head (h + 1);
        Some v
      )
    )
  }

let size q =
  Xt.commit {
    tx = (fun ~xt ->
      let h = Xt.get ~xt q.head in
      let t = Xt.get ~xt q.tail in
      t - h
    )
  }