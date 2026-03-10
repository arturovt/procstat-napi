import test from 'ava';
import childProcess from 'node:child_process';

import { getContextSwitches } from '../index.mjs';

test('gets context switches', async (t) => {
  const child = childProcess.spawn('node', ['./tests/fixtures/event-loop.js']);
  await new Promise((resolve) => setTimeout(resolve, 500));
  const result = getContextSwitches(child.pid);
  child.kill();
  t.truthy(result.voluntary > 1);
});
