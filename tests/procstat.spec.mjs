import test from 'ava';

import { createMonitor } from '../index.mjs';

test('gets context switches', async (t) => {
  const monitor = createMonitor({ intervalMs: 1000 });
  const recorder = [];
  monitor.start((stats) => {
    recorder.push(stats);
  });
  const timeoutId = setInterval(() => {}, 1);
  await new Promise((resolve) => setTimeout(resolve, 2000));
  clearInterval(timeoutId);
  monitor.stop();
  t.truthy(recorder.length > 0);
  t.truthy(recorder[0].voluntaryContextSwitches > 0);
});
