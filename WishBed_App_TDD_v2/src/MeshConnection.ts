/**
 * MeshConnection Mock - Simulates mesh network connection
 * Used for testing privacy compliance and Emergency Stop
 */

import { EventBus } from './EventBus';
import { MeshMessage } from './types';

export interface MeshState {
  isConnected: boolean;
  peerId: string | null;
  sentMessages: MeshMessage[];
}

class MeshConnectionClass {
  private state: MeshState = {
    isConnected: false,
    peerId: null,
    sentMessages: [],
  };

  private disconnectedTimestamp: number = 0;

  constructor() {
    this.setupEventListeners();
  }

  private setupEventListeners(): void {
    EventBus.on('EMERGENCY_STOP', () => {
      this.disconnectedTimestamp = performance.now();
      this.disconnect();
    });

    EventBus.on('DISCONNECT_MESH', () => {
      this.disconnectedTimestamp = performance.now();
      this.disconnect();
    });
  }

  connect(peerId: string): void {
    this.state.isConnected = true;
    this.state.peerId = peerId;
  }

  disconnect(): void {
    this.state.isConnected = false;
    this.state.peerId = null;
  }

  sendMessage(message: MeshMessage): void {
    if (this.state.isConnected) {
      this.state.sentMessages.push(message);
    }
  }

  getState(): MeshState {
    return {
      ...this.state,
      sentMessages: [...this.state.sentMessages],
    };
  }

  getSentMessages(): MeshMessage[] {
    return [...this.state.sentMessages];
  }

  getDisconnectedTimestamp(): number {
    return this.disconnectedTimestamp;
  }

  hasPersonalDataInMessages(): boolean {
    const personalDataPatterns = [
      /email/i,
      /password/i,
      /phone/i,
      /address/i,
      /location/i,
      /ip.?address/i,
      /device.?id/i,
    ];

    for (const msg of this.state.sentMessages) {
      const payloadStr = JSON.stringify(msg.payload);
      for (const pattern of personalDataPatterns) {
        if (pattern.test(payloadStr)) {
          return true;
        }
      }
    }
    return false;
  }

  reset(): void {
    this.state = {
      isConnected: false,
      peerId: null,
      sentMessages: [],
    };
    this.disconnectedTimestamp = 0;
    // Re-register event listeners after reset
    this.setupEventListeners();
  }
}

export const MeshConnection = new MeshConnectionClass();
