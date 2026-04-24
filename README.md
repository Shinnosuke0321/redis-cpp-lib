# 🚀 Actor-Based Async Redis Client (C++)

A **high-performance, event-driven Redis client** built from scratch in modern C++, designed around an **actor model** to eliminate data races and simplify concurrency.

> 💡 This project explores how production-grade async systems (like Node.js or Tokio) are built under the hood.

---

## ⚡ Highlights

- 🧠 **Actor-based architecture** → no shared mutable state, no locks
- ⚡ **Fully asynchronous** → non-blocking I/O using `libevent` + `hiredis`
- 🧵 **Thread-safe API** → safe to call from multiple threads
- 🧱 **Command pattern design** → extensible and composable operations
- 🛑 **Typed error system** → structured error propagation using `std::expected`
- 🔄 **Deterministic execution model** → all logic serialized in a single event loop

---

## 🏗️ Architecture (What Makes This Interesting)

``` id="2e0u2p"
Multi-threaded API
        ↓
Message passing (post)
        ↓
Single-threaded Actor
        ↓
Event-driven Transport (libevent + hiredis)
        ↓
Redis
```

### Key Idea

> Instead of using locks, this client **serializes all operations through an actor running on a single event loop thread**.

This achieves:
- Zero data races
- No mutex contention
- Predictable execution
---
## How It Works

### Execution Flow

``` id="x1kg1j"
client.get("key")
   ↓
post() to event loop
   ↓
Actor queues command
   ↓
Transport sends request (hiredis)
   ↓
Callback resolves result
   ↓
Future completes
```
---
### Connection Lifecycle

``` id="d7zn6g"
connect()
   ↓
redisAsyncConnect
   ↓
AUTH (optional)
   ↓
SELECT DB (optional)
   ↓
Ready
```

---

## Concurrency Model

Unlike traditional clients:

``` id="q8o9v1"
thread-per-request
shared state + mutexes
```

This client uses:

``` id="m3plk8"
✔ single-threaded actor
✔ message passing
✔ event-driven execution
```

👉 Multiple threads can safely call the client, but **all work is executed in a single, serialized context**.

---

## Error Handling

Custom typed error system:

```cpp id="k92v0z"
std::expected<T, error::exception>
```

- Strongly-typed error categories
- Clear propagation across async boundaries
- No exceptions required

---

## 🧪 What I Focused On

This project is not just about Redis—it’s about **systems design**:

- Designing a **lock-free concurrency model**
- Building an **event-driven runtime**
- Implementing a **command execution pipeline**
- Handling **async error propagation correctly**
- Debugging real-world issues (deadlocks, event loop stalls, memory ownership)

---

## 🔥 Challenges Solved

- Eliminating deadlocks in event loop scheduling
- Correctly integrating `libevent` with async hiredis
- Managing lifetime of async callbacks safely
- Ensuring no response mix-ups under concurrency
- Designing a reusable error abstraction across layers

---

## Future Work

- Connection pooling
- Command pipelining
- Streaming responses
- PostgreSQL client built on the same architecture

---

## 🏁 Takeaway

> This project is essentially a **mini async runtime + Redis client**, built to understand how modern high-performance systems actually work.

--