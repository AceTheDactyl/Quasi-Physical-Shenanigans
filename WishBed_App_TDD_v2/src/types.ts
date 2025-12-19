/**
 * WishBed UCF System Types
 * Core type definitions for the Safety Layer and related modules
 */

export type ConsentType = 'SESSION' | 'MICROPHONE' | 'RECORDING' | 'MESH' | 'ANALYTICS';
export type ImpactLevel = 'LOW' | 'HIGH' | 'SYSTEM';
export type ScanResult = 'PASS' | 'WARN' | 'REJECT';
export type SessionState = 'IDLE' | 'ACTIVE' | 'PAUSED' | 'EMERGENCY_STOPPED';

export interface SafetyCaps {
  brightness: number;      // 0.0 - 1.0
  motionVelocity: number;  // 0.0 - 2.0
  flickerHz: number;       // 0 - 30
  particleDensity: number; // 0.0 - 1.0
}

export interface ConsentState {
  session: boolean;
  microphone: boolean;
  recording: boolean;
  mesh: boolean;
  analytics: boolean;
  timestamps: Map<ConsentType, Date>;
}

export interface ScanReport {
  result: ScanResult;
  violations: string[];
  suggestions: string[];
}

export interface Action {
  type: ActionType;
  payload?: unknown;
  dangerous?: boolean;
}

export type ActionType =
  | 'START_SESSION'
  | 'END_SESSION'
  | 'SHARE_TO_MESH'
  | 'PUBLISH_BLUEPRINT'
  | 'DELETE_DATA'
  | 'EMERGENCY_STOP'
  | 'ENABLE_VOICE'
  | 'START_RECORDING'
  | 'JOIN_MESH'
  | 'UPDATE_SETTINGS'
  | 'VIEW_CONTENT'
  | 'DANGEROUS_FUNCTION';

export interface Blueprint {
  id: string;
  name: string;
  description: string;
  stages: BlueprintStage[];
}

export interface BlueprintStage {
  name: string;
  narration: string;
  duration: number;
}

export interface OperatorPersona {
  id: string;
  name: string;
  narration: string[];
}

export interface ImportArchive {
  blueprints: Blueprint[];
  personas: OperatorPersona[];
}

export interface NetworkCall {
  url: string;
  method: string;
  timestamp: number;
  data?: unknown;
}

export interface MeshMessage {
  type: string;
  payload: unknown;
  sender: string;
}
