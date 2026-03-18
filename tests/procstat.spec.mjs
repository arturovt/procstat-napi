import test from 'ava';

import { createMonitor } from '../index.mjs';

test('gets context switches', async (t) => {
  const monitor = createMonitor({ intervalMs: 1000 });
  const recorder = [];
  const onStats = (stats) => recorder.push(stats);
  monitor.on('stats', onStats);
  const timeoutId = setInterval(() => {}, 1);
  await new Promise((resolve) => setTimeout(resolve, 2000));
  clearInterval(timeoutId);
  monitor.off('stats', onStats);
  t.truthy(recorder.length > 0);
  t.truthy(recorder[0].voluntaryContextSwitches > 0);
});
