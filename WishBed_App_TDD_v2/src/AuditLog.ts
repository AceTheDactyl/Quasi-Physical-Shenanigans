/**
 * AuditLog - Security audit logging for safety-critical events
 */

export interface AuditEntry {
  action: string;
  timestamp: number;
  details?: unknown;
}

class AuditLogClass {
  private entries: AuditEntry[] = [];

  record(action: string, timestamp: number, details?: unknown): void {
    this.entries.push({ action, timestamp, details });
  }

  getEntries(): AuditEntry[] {
    return [...this.entries];
  }

  getEntriesByAction(action: string): AuditEntry[] {
    return this.entries.filter(e => e.action === action);
  }

  clear(): void {
    this.entries = [];
  }
}

export const AuditLog = new AuditLogClass();
