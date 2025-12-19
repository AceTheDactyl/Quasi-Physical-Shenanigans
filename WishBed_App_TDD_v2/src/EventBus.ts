/**
 * EventBus - Simple event emitter for system-wide events
 */

type EventHandler = (...args: unknown[]) => void;

class EventBusClass {
  private handlers: Map<string, Set<EventHandler>> = new Map();
  private eventLog: Array<{ event: string; timestamp: number }> = [];

  emit(event: string, ...args: unknown[]): void {
    this.eventLog.push({ event, timestamp: performance.now() });
    const handlers = this.handlers.get(event);
    if (handlers) {
      handlers.forEach(handler => handler(...args));
    }
  }

  on(event: string, handler: EventHandler): () => void {
    if (!this.handlers.has(event)) {
      this.handlers.set(event, new Set());
    }
    this.handlers.get(event)!.add(handler);

    // Return unsubscribe function
    return () => {
      this.handlers.get(event)?.delete(handler);
    };
  }

  off(event: string, handler: EventHandler): void {
    this.handlers.get(event)?.delete(handler);
  }

  clear(): void {
    this.handlers.clear();
    this.eventLog = [];
  }

  getEventLog(): Array<{ event: string; timestamp: number }> {
    return [...this.eventLog];
  }

  getLastEventTimestamp(event: string): number | undefined {
    for (let i = this.eventLog.length - 1; i >= 0; i--) {
      if (this.eventLog[i].event === event) {
        return this.eventLog[i].timestamp;
      }
    }
    return undefined;
  }
}

export const EventBus = new EventBusClass();
