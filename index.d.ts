interface ContextSwitches {
  voluntary: number;
  nonvoluntary: number;
}

export function getContextSwitches(pid: number): ContextSwitches;
