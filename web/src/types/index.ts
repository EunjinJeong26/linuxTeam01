// ---- 서버 응답 raw 타입 (명세서 5.1) ----

export interface ApiLoginResponse {
  token: string;
  user_id: number;
  username: string;
}

export interface ApiRegisterResponse {
  user_id: number;
  username: string;
}

export interface ApiMemberInfo {
  user_id: number;
  username: string;
  role: 'leader' | 'member';
}

export interface ApiMyTeamResponse {
  team_id: number;
  name: string;
  invite_code: string;
  members: ApiMemberInfo[];
}

export interface ApiLog {
  logId: number;
  userId: number;
  teamId: number;
  session: string; // 'YYYY-MM-DDTHH:00'
  cpuUsage: number;
  memUsedGb: number;
  memTotalGb: number;
  branch: string | null;
  comment: string;
  createdAt: string; // 'YYYY-MM-DDTHH:mm:ss'
  commits: string[];
}

// ---- 프론트 내부 모델 (명세서 5.2) ----

export interface User {
  userId: number;
  username: string;
}

export interface Team {
  teamId: number;
  teamName: string;
  inviteCode: string;
}

export interface Member {
  userId: number;
  username: string;
  role: 'leader' | 'member';
}

export interface Log {
  logId: number;
  userId: number;
  username: string;
  teamId: number;
  sessionDate: string; // 'YYYY-MM-DD'
  sessionHour: number; // 0–23
  cpuUsage: number; // %
  memPercent: number; // %
  memUsedGb: number;
  memTotalGb: number;
  branch: string | null;
  commitMessages: string[];
  comment: string;
  createdAt: string;
}
