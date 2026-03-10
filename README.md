# procstat-napi

Node.js N-API bindings for reading Linux process statistics from `/proc`. Returns context switches (voluntary/nonvoluntary) for any process by PID. Zero dependencies, synchronous, negligible overhead.

> **Note:** On non-Linux platforms (macOS, Windows) all values are returned as `0`.

## Installation
```sh
npm install procstat-napi
# or
yarn add procstat-napi
# or
pnpm install procstat-napi
```

## Usage
```js
import { getContextSwitches } from 'procstat-napi';

const { voluntary, nonvoluntary } = getContextSwitches(process.pid);

console.log({ voluntary, nonvoluntary });
// { voluntary: 84603, nonvoluntary: 156 }
```

### Context switches explained

- **voluntary** — the process yielded the CPU willingly (waiting on I/O, timers, etc.). Normal and expected in event-driven apps.
- **nonvoluntary** — the OS forcibly preempted the process. High values indicate CPU saturation.

A nonvoluntary/voluntary ratio above ~5% is worth investigating.
