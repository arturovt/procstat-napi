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

const onStats = (stats) => {
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
};

monitor.on('stats', onStats);

// later
monitor.off('stats', onStats);
```

## ASan leak reporting

When built with AddressSanitizer, the monitor can deliver leak reports directly to JavaScript instead of writing them to stderr.

```js
const monitor = createMonitor({ intervalMs: 1000 });

monitor.on('leak', (report) => {
  console.error('ASan leak detected:\n', report);
});
```

The report string is the full ASan output. The callback fires on the Node.js event loop, so it is safe to call any JS from inside it.

To build with ASan:
```sh
npm run build
```

To run with ASan:
```sh
LD_PRELOAD=$(gcc -print-file-name=libasan.so) NODE_OPTIONS="--expose-gc" node your_script.js
```

## API

### `createMonitor(options): Monitor`

`options` is required.

| Option | Type | Description |
|---|---|---|
| `intervalMs` | `number` | Polling interval in milliseconds |

### `monitor.on(event, callback)`

| Event | Callback signature | Description |
|---|---|---|
| `'stats'` | `(stats: Stats) => void` | Fires on each timer tick. Starts the timer on first call |
| `'leak'` | `(report: string) => void` | Fires when ASan detects a leak. ASan build only |

### `monitor.off(event, callback)`

Removes the specific `callback` for the given event. Stops the timer when the last `stats` listener is removed. Tears down the ASan async handle when the last `leak` listener is removed across all monitors.

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

An `involuntaryContextSwitches / voluntaryContextSwitches` ratio above ~5% is worth investigating.
