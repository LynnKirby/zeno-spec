# Rationale notes

In the future there will be a *Zeno Language Rationale* document explaining language design decisions.

For now these are notes ad thought dumps.

## Principles

1. Correctness.
2. Efficient access to platform resources.

When adding a new feature, consider the effect on:

- Users of software written in Zeno.
- Language users.
- Tool writers, especially those outside the core team.
  (Easy for me/us doesn't mean easy for others.)
- Formal semantics.

You must:
- Explain the feature to give programmers an intuitive understanding of it.
- Provide a straightforward implementation of the feature.
- Formalize the feature.

## Parameter modes

Parameters have modes like `in`, `out`, `mut`.

Parameters are passed by reference unless the `move` mode is used.

### Benefit: ease of use

Modes are easy to conceptualize.
They say directly how the passed value can be used.

### Benefit: guaranteed calling convention optimizations

These signatures could be compiled the same way to pass the result in a register:

```
def with_ret() -> Int32
def with_out(out result: Int32)
```

If instead the out parameter was a reference or pointer, the calling convention *must* allocate a memory location and pass the pointer to it.

### Downside: poor integration with generics

A purely type-based approach like Rust integrates easily into parametric polymorphism because the different ways to use parameters are just types.
To get the same integration with parameter modes, modes must be included as a kind of generic parameter.
This complicates the type system and how generic algorithms are written.

In the example below, when modes are not generic parameters, this definition only works on functions with `in T` parameters. But in Rust this works for any type, including references.

```
def call_twice[F, T](f: F, t: T)
    where F : Fn(T) -> void
{
    f(t);
    f(t);
}
```

## Memory model

Weak memory model using viewfronts [[Podkopaev2018](#ref-Podkopaev2018)], similar to [[Dalvandi2020](#ref-Dalvandi2020)].

### Open question: mixed-size operations

See [[Flur2017](#ref-Flur2017)].

How to support mixed-size operations?
Load from multiple stores.
Load from a larger store.
And combined.

Basic idea, extend reads to use a set of writes:
- For each byte, select a write that covers it.
- Consistency requirement.
  For each write, timestamp of the read byte must be greater than the timestamp in the thread view.
- Nondeterministically order the set of writes.
  The value of each write is applied in order.
  In a set of equal sized writes, each application overwrites the entire value (the last write wins).
  In a set with mixed size writes, a write may partially overwrite the result, ending up with a torn read.

Merging view of relaxed read: for each byte in the result, merge the timestamp of the byte in the source write.

Merging view of acquire read: for each byte in the result, merge the view of the source write.

### Open question: should we have fences?

Fences can be described operationally by having an acquire and a release view per thread in addition to the current view [[Kang2017](#ref-Kang2017)].
Problem: it makes the operational semantics more complex.
Are fences worth the added complexity?

### Open question: should we have SC operations?

[[Lahav2017](#ref-Lahav2017)] showed that the standard SC instruction sequences for ARM and Power are unsound under the C11/C++11 model.
The recommended fix was to change the definition of SC, and the standards followed it.

I would have kept the definition and changed the instruction sequences.
In general this requires performing writes with a compare-exchange plus appropriate fences (see the atomic accesses in [[Dolan2018](#ref-Dolan2018)]).

There are still some questions here:
- How to define SC operations in the mixed-size model?
  Is there a definition that can be implemented by a hardware memory model?
- Will future weak memory models even provide the instructions to get "real" SC?
  Given that C/C++ no longer support it, it's possible that future instructions sets won't.
- Are SC atomics worth including?
  The cost is non-zero and it's unclear to me if the benefit is enough.
  Atomic algorithms nowadays are generally designed for acquire-release in the first place.

### Load buffering

The memory model should preserve load-to-store ordering (it disallows load buffering).
For weak memory models like ARM and Power there are two basic methods [[Boehm2014](#ref-Boehm2014)]:

- Fence before store (FBS) uses a load-store fence.

- Branch after load (BAL) creates a control dependency on the load which prevents stores after the branch from being speculatively executed.
  Most but not all weak memory architectures support this.

At first glance it seems FBS is the better choice because loads occur more frequently than stores.
Benchmarks of each scheme for OCaml shows BAL is cheaper for Power compared to ARM [[Dolan2018](#ref-Dolan2018)].
This makes sense because OCaml uses this scheme for all regular accesses to mutable fields and `lwsync` is stronger than `dmb ld` (prevents store-store reorderings in addition to load-load and load-store).

### ARMv7 mapping

| Operation     | Implementation        |
|---------------|-----------------------|
| relaxed load  | `ldr`                 |
| relaxed store | `dmb ish; str`        |
| acquire load  | `ldr; dmb ish`        |
| release store | `dmb ish; str`        |

### AArch32 mapping

| Operation     | Implementation        |
|---------------|-----------------------|
| relaxed load  | `ldr`                 |
| relaxed store | `dmb ishld; str`      |
| acquire load  | `lda`                 |
| release store | `stl`                 |

### AArch64 mapping

| Operation     | Implementation        |
|---------------|-----------------------|
| relaxed load  | `ldr`                 |
| relaxed store | `dmb ishld; str`      |
| acquire load  | `ldar`                |
| release store | `stlr`                |

### Alpha mapping

Alpha only has a full fence (`mb`) and a store-store fence (`wmb`).
Unlike most memory models, control dependencies aren't respected and we can't use BAL.

| Operation       | Implementation |
|-----------------|----------------|
| relaxed load    | `ldq`          |
| relaxed store   | `mb; stq`      |
| acquire load    | `ldq; mb`      |
| release store   | `mb; stq`      |

### Power mapping

| Operation         | Implementation             |
|-------------------|----------------------------|
| FBS relaxed load  | `ld`                       |
| FBS relaxed store | `lwsync; st`               |
| BAL relaxed load  | `ld; cmp; bc L; L:`        |
| BAL relaxed store | `st`                       |
| acquire load      | `ld; cmp; bc L; L:; isync` |
| release store     | `lwsync; st`               |

### RISC-V mapping

RISC-V has a load-store fence (`fence r, w`).

| Operation     | Implementation    |
|---------------|-------------------|
| relaxed load  | `ld`              |
| relaxed store | `fence r, w; sd`  |
| acquire load  | `ld; fence r, rw` |
| release store | `fence rw, r; sd` |

## References

- <a id="ref-Boehm2014"></a> **[Boehm2014]**
  Hans-J. Boehm and Brian Demsky.
  2014.
  Outlawing ghosts: avoiding out-of-thin-air results.
  In *Proceedings of the workshop on Memory Systems Performance and Correctness* (MSPC 2014).
  https://doi.org/10.1145/2618128.2618134

- <a id="ref-Dalvandi2020"></a> **[Dalvandi2020]**
  Sadegh Dalvandi, Simon Doherty, Brijesh Dongol, and Heike Wehrheim.
  2020.
  Owicki-Gries reasoning for C11 RAR.
  In *34th European Conference on Object-Oriented Programming* (ECOOP 2020).
  https://doi.org/10.4230/LIPIcs.ECOOP.2020.11

- <a id="ref-Dolan2018"></a> **[Dolan2018]**
  Stephen Dolan, KC Sivaramakrishnan, and Anil Madhavapeddy.
  2018.
  Bounding data races in space and time.
  In *Proceedings of the 39th ACM SIGPLAN Conference on Programming Language Design and Implementation* (PLDI 2018).
  https://doi.org/10.1145/3192366.3192421.
  Available at: https://kcsrk.info/papers/pldi18-memory.pdf

- <a id="ref-Flur2017"></a> **[Flur2017]**
  Shaked Flur, Susmit Sarkar, Christopher Pulte, Kyndylan Nienhuis, Luc Maranget, Kathryn E. Gray, Ali Sezgin, Mark Batty, and Peter Sewell.
  2017.
  Mixed-size concurrency: ARM, POWER, C/C++11, and SC.
  In *Proceedings of the 44th ACM SIGPLAN Symposium on Principles of Programming Languages* (POPL 2017).
  https://doi.org/10.1145/3009837.3009839.
  Available at: https://inria.hal.science/hal-01413221

- <a id="ref-Kang2017"></a> **[Kang2017]**
  Jeehoon Kang, Chung-Kil Hur, Ori Lahav, Viktor Vafeiadis, and Derek Dreyer.
  2017.
  A promising semantics for relaxed-memory concurrency.
  In *Proceedings of the 44th ACM SIGPLAN Symposium on Principles of Programming Languages* (POPL 2017).
  https://doi.org/10.1145/3009837.3009850

- <a id="ref-Lahav2017"></a> **[Lahav2017]**
  Ori Lahav, Viktor Vafeiadis, Jeehoon Kang, Chung-Kil Hur, and Derek Dreyer.
  2017.
  Repairing sequential consistency in C/C++11.
  In *Proceedings of the 38th ACM SIGPLAN Conference on Programming Language Design and Implementation* (PLDI 2017).
  https://doi.org/10.1145/3062341.3062352

- <a id="ref-Podkopaev2018"></a> **[Podkopaev2018]**
  Anton Podkopaev, Ilya Sergey, and Aleksandar Nanevski. 2018.
  Operational aspects of C/C++ concurrency.
  https://arxiv.org/abs/1606.01400
