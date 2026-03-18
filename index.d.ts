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
  on(event: 'stats', callback: (stats: Stats) => void): void;
  on(event: 'leak', callback: (report: string) => void): void;

  off(event: 'stats', callback: (stats: Stats) => void): void;
  off(event: 'leak', callback: (report: string) => void): void;
}

export function createMonitor(options: MonitorOptions): Monitor;
