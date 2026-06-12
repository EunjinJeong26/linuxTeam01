import { mapLog, mapTeam } from './mappers';
import type { ApiLog, ApiMyTeamResponse } from '../types';

const apiLog: ApiLog = {
  logId: 109,
  userId: 1,
  teamId: 1,
  session: '2026-06-12T09:00',
  cpuUsage: 42,
  memUsedGb: 5.2,
  memTotalGb: 16,
  branch: 'main',
  comment: '테스트 코멘트',
  createdAt: '2026-06-12T09:07:23',
  commits: ['fix: 버그 수정'],
};

describe('mapLog', () => {
  const names = new Map<number, string>([[1, 'jin']]);

  it('session을 날짜/시각으로 분리하고 username을 join한다', () => {
    const log = mapLog(apiLog, names);
    expect(log.sessionDate).toBe('2026-06-12');
    expect(log.sessionHour).toBe(9);
    expect(log.username).toBe('jin');
  });

  it('memPercent를 반올림 정수로 계산한다', () => {
    const log = mapLog(apiLog, names);
    expect(log.memPercent).toBe(33); // 5.2 / 16 = 32.5%
  });

  it('멤버 목록에 없는 userId는 unknown으로 표시한다', () => {
    const log = mapLog({ ...apiLog, userId: 7 }, names);
    expect(log.username).toBe('unknown(7)');
  });
});

describe('mapTeam', () => {
  it('snake_case 필드를 내부 모델로 변환한다', () => {
    const api: ApiMyTeamResponse = {
      team_id: 1,
      name: 'linux01',
      invite_code: 'A1B2C3D4',
      members: [{ user_id: 1, username: 'jin', role: 'leader' }],
    };
    const { team, members } = mapTeam(api);
    expect(team).toEqual({ teamId: 1, teamName: 'linux01', inviteCode: 'A1B2C3D4' });
    expect(members[0]).toEqual({ userId: 1, username: 'jin', role: 'leader' });
  });
});
