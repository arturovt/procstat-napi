export interface MonitorOptions {
  intervalMs: number;
}

export interface Stats {
  // Context switches
  voluntaryContextSwitches: number;
  involuntaryContextSwitches: number;
  // CPU times in seconds
  userCpuTime: number;
  systemCpuTime: number;
  // Memory
  maxRss: number;
  // Page faults
  minorFaults: number;
  majorFaults: number;
  // I/O
  blockInputOps: number;
  blockOutputOps: number;
}

export interface Monitor {
  start(callback: (stats: Stats) => void): void;
  stop(): void;
}

export function createMonitor(options: MonitorOptions): Monitor;
