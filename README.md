# procstat-napi

Node.js N-API bindings for reading process statistics via `getrusage`. Returns context switches, CPU times, memory, page faults, and I/O metrics. Zero dependencies, negligible overhead.

> **Note:** Linux and macOS only. Windows is not supported.

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
import { createMonitor } from 'procstat-napi';

const monitor = createMonitor({ intervalMs: 1000 });

monitor.start((stats) => {
  console.log(stats);
  // {
  //   voluntaryContextSwitches: 84603,
  //   involuntaryContextSwitches: 156,
  //   userCpuTime: 1.23,
  //   systemCpuTime: 0.45,
  //   maxRss: 204800,
  //   minorFaults: 300,
  //   majorFaults: 2,
  //   blockInputOps: 10,
  //   blockOutputOps: 5
  // }
});

monitor.stop();
```

## API

### `createMonitor(options?): Monitor`

| Option | Type | Default | Description |
|---|---|---|---|
| `intervalMs` | `number` | `1000` | Polling interval in milliseconds |

### `monitor.start(callback)`

Starts the timer and calls `callback` with a `Stats` object on each tick.

### `monitor.stop()`

Stops the timer and releases the handle.

## Metrics explained

| Field | Description |
|---|---|
| `voluntaryContextSwitches` | Process yielded CPU willingly (I/O, timers). Normal in event-driven apps |
| `involuntaryContextSwitches` | OS forcibly preempted the process. High values indicate CPU saturation |
| `userCpuTime` | Time spent in user space (seconds) |
| `systemCpuTime` | Time spent in kernel space (seconds) |
| `maxRss` | Peak resident set size (KB) |
| `minorFaults` | Page faults served without disk I/O |
| `majorFaults` | Page faults requiring disk I/O |
| `blockInputOps` | Block input operations |
| `blockOutputOps` | Block output operations |

A `involuntaryContextSwitches / voluntaryContextSwitches` ratio above ~5% is worth investigating.
