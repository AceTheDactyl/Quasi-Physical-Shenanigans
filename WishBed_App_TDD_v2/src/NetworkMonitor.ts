/**
 * NetworkMonitor - Monitors and records network activity
 * Used for privacy compliance testing
 */

import { NetworkCall } from './types';

class NetworkMonitorClass {
  private calls: NetworkCall[] = [];
  private enabled: boolean = false;
  private allowedDomains: Set<string> = new Set();

  enable(): void {
    this.enabled = true;
    this.calls = [];
  }

  disable(): void {
    this.enabled = false;
  }

  isEnabled(): boolean {
    return this.enabled;
  }

  setAllowedDomains(domains: string[]): void {
    this.allowedDomains = new Set(domains);
  }

  recordCall(url: string, method: string, data?: unknown): void {
    if (this.enabled) {
      this.calls.push({
        url,
        method,
        timestamp: Date.now(),
        data,
      });
    }
  }

  getCalls(): NetworkCall[] {
    return [...this.calls];
  }

  getUnexpectedCalls(): NetworkCall[] {
    return this.calls.filter(call => {
      const domain = this.extractDomain(call.url);
      return !this.allowedDomains.has(domain);
    });
  }

  hasPersonalDataInCalls(): boolean {
    const personalDataPatterns = [
      /email/i,
      /password/i,
      /ssn/i,
      /credit.?card/i,
      /phone/i,
      /address/i,
      /user.?id/i,
      /session.?id/i,
      /location/i,
      /gps/i,
    ];

    for (const call of this.calls) {
      const dataStr = JSON.stringify(call.data || '');
      for (const pattern of personalDataPatterns) {
        if (pattern.test(dataStr) || pattern.test(call.url)) {
          return true;
        }
      }
    }
    return false;
  }

  private extractDomain(url: string): string {
    try {
      return new URL(url).hostname;
    } catch {
      return url;
    }
  }

  clear(): void {
    this.calls = [];
  }
}

export const NetworkMonitor = new NetworkMonitorClass();
