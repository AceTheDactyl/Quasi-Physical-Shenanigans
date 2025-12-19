/**
 * AudioOutput Mock - Simulates the audio output system
 * Used for testing Emergency Stop compliance
 */

import { EventBus } from './EventBus';

export interface AudioState {
  isPlaying: boolean;
  volume: number;
  frequency: number;
}

class AudioOutputClass {
  private state: AudioState = {
    isPlaying: false,
    volume: 0.5,
    frequency: 432,
  };

  private stoppedTimestamp: number = 0;

  constructor() {
    this.setupEventListeners();
  }

  private setupEventListeners(): void {
    EventBus.on('EMERGENCY_STOP', () => {
      this.stoppedTimestamp = performance.now();
      this.stopAllOutput();
    });

    EventBus.on('HALT_AUDIO', () => {
      this.stoppedTimestamp = performance.now();
      this.stopAllOutput();
    });
  }

  play(frequency: number = 432, volume: number = 0.5): void {
    this.state.isPlaying = true;
    this.state.frequency = frequency;
    this.state.volume = volume;
  }

  stop(): void {
    this.state.isPlaying = false;
  }

  stopAllOutput(): void {
    this.state.isPlaying = false;
    this.state.volume = 0;
    this.state.frequency = 0;
  }

  getState(): AudioState {
    return { ...this.state };
  }

  getStoppedTimestamp(): number {
    return this.stoppedTimestamp;
  }

  isOutputStopped(): boolean {
    return !this.state.isPlaying && this.state.volume === 0;
  }

  reset(): void {
    this.state = {
      isPlaying: false,
      volume: 0.5,
      frequency: 432,
    };
    this.stoppedTimestamp = 0;
    // Re-register event listeners after reset
    this.setupEventListeners();
  }
}

export const AudioOutput = new AudioOutputClass();
