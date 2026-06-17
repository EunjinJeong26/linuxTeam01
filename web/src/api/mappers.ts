import type { ApiLog, ApiMyTeamResponse, Log, Member, Team } from '../types';
import { splitSession } from '../utils/date';

export function mapTeam(api: ApiMyTeamResponse): { team: Team; members: Member[] } {
  return {
    team: {
      teamId: api.team_id,
      teamName: api.name,
      inviteCode: api.invite_code,
    },
    members: api.members.map((m) => ({
      userId: m.user_id,
      username: m.username,
      role: m.role,
    })),
  };
}

export function mapLog(api: ApiLog, usernameOf: ReadonlyMap<number, string>): Log {
  const { date, hour } = splitSession(api.session);
  return {
    logId: api.logId,
    userId: api.userId,
    username: usernameOf.get(api.userId) ?? `unknown(${api.userId})`,
    teamId: api.teamId,
    sessionDate: date,
    sessionHour: hour,
    cpuUsage: Math.round(api.cpuUsage),
    memPercent: Math.round((api.memUsedGb / api.memTotalGb) * 100),
    memUsedGb: api.memUsedGb,
    memTotalGb: api.memTotalGb,
    branch: api.branch,
    commitMessages: api.commits,
    comment: api.comment,
    createdAt: api.createdAt,
  };
}
