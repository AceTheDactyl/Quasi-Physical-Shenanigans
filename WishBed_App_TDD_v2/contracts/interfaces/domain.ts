export type LaneType = "Simulation" | "Interface";
export type OverlayMode = "Bed" | "Walking";

export type OverlayLinkStatus =
  | "Disconnected"
  | "Scanning"
  | "Binding"
  | "Calibrating"
  | "Active"
  | "Faulted";

export interface ConsentState {
  accepted: boolean;
  acceptedAt?: string; // ISO timestamp
  scope: {
    microphone?: boolean;
    recording?: boolean;
    mesh?: boolean;
    marketplace?: boolean;
  };
}

export interface SafetyCaps {
  brightnessCap: number; // 0..1
  motionCap: number;     // 0..1
  flickerCapHz: number;  // e.g. <= 3
  particleDensityCap: number; // 0..1
}

export interface SessionState {
  sessionId: string;
  mode: OverlayMode;
  lane: LaneType;
  status: "Idle" | "Initializing" | "Active" | "Integrating" | "Shutdown";
  overlayLinkStatus: OverlayLinkStatus;
  activeArchetype?: string;
  activeProfile?: string;
  safetyCaps: SafetyCaps;
  consent: ConsentState;
}

export interface Blueprint {
  blueprintId: string;
  name: string;
  version: string;
  appearance: Record<string, unknown>;
  profiles: string[];
  safetyCaps?: Partial<SafetyCaps>;
  ritualPack?: Record<string, unknown>; // symbolic scripts (must be labeled)
  operatorPersona?: Record<string, unknown>; // tone-only
}

export interface VoiceIntent {
  intentId: string;
  phrase: string;
  confidence: number; // 0..1
  impact: "Informational" | "Low" | "Medium" | "High" | "Emergency";
  requiresConfirmation: boolean;
  mappedAction: string;
}
