/**
 * Renderer Mock - Simulates the visual rendering system
 * Used for testing SafeMinimalMode and Emergency Stop compliance
 */

import { EventBus } from './EventBus';

export interface RenderState {
  isActive: boolean;
  brightness: number;
  motionVelocity: number;
  flickerHz: number;
  particleDensity: number;
  complexVisualsEnabled: boolean;
}

class RendererClass {
  private state: RenderState = {
    isActive: false,
    brightness: 0.8,
    motionVelocity: 1.0,
    flickerHz: 10,
    particleDensity: 0.7,
    complexVisualsEnabled: true,
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

    EventBus.on('HALT_VISUAL', () => {
      this.stoppedTimestamp = performance.now();
      this.stopAllOutput();
    });

    EventBus.on('SAFE_MINIMAL_MODE', () => {
      this.stoppedTimestamp = performance.now();
      this.enterMinimalMode();
    });

    EventBus.on('EXIT_SAFE_MINIMAL_MODE', () => {
      this.exitMinimalMode();
    });
  }

  start(): void {
    this.state.isActive = true;
    this.state.complexVisualsEnabled = true;
  }

  stopAllOutput(): void {
    this.state.isActive = false;
    this.state.brightness = 0;
    this.state.motionVelocity = 0;
    this.state.flickerHz = 0;
    this.state.particleDensity = 0;
    this.state.complexVisualsEnabled = false;
  }

  enterMinimalMode(): void {
    this.state.brightness = 0.3;
    this.state.motionVelocity = 0.2;
    this.state.flickerHz = 0;
    this.state.particleDensity = 0.1;
    this.state.complexVisualsEnabled = false;
  }

  exitMinimalMode(): void {
    this.state.brightness = 0.8;
    this.state.motionVelocity = 1.0;
    this.state.flickerHz = 10;
    this.state.particleDensity = 0.7;
    this.state.complexVisualsEnabled = true;
  }

  getState(): RenderState {
    return { ...this.state };
  }

  getStoppedTimestamp(): number {
    return this.stoppedTimestamp;
  }

  isOutputStopped(): boolean {
    return (
      !this.state.isActive &&
      this.state.brightness === 0 &&
      this.state.motionVelocity === 0 &&
      this.state.flickerHz === 0 &&
      this.state.particleDensity === 0
    );
  }

  areComplexVisualsStopped(): boolean {
    return !this.state.complexVisualsEnabled;
  }

  reset(): void {
    this.state = {
      isActive: false,
      brightness: 0.8,
      motionVelocity: 1.0,
      flickerHz: 10,
      particleDensity: 0.7,
      complexVisualsEnabled: true,
    };
    this.stoppedTimestamp = 0;
    // Re-register event listeners after reset
    this.setupEventListeners();
  }
}

export const Renderer = new RendererClass();
