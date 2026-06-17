import type { ApiLog, ApiMyTeamResponse } from '../../types';
import { pad2, todayStr } from '../../utils/date';

const DELAY_MS = 300;

export function delay(): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, DELAY_MS));
}

export const mockTeam: ApiMyTeamResponse = {
  team_id: 1,
  name: 'linux01',
  invite_code: 'A1B2C3D4',
  members: [
    { user_id: 1, username: 'jin', role: 'leader' },
    { user_id: 2, username: 'minseo', role: 'member' },
    { user_id: 3, username: 'hyun', role: 'member' },
    { user_id: 4, username: 'sora', role: 'member' },
  ],
};

const COMMENTS = [
  'API 클라이언트 모듈 리팩토링 중',
  '로그 파싱 버그 수정하고 테스트 추가했음',
  'UDS 소켓 연결 오류 디버깅하는 중',
  '타임라인 그리드 렌더링 작업',
  'JWT 인터셉터 붙이고 401 처리 확인',
  '문서 업데이트하고 PR 올렸습니다',
  '세션 중복 체크 로직 손보는 중',
];

const BRANCHES = ['main', 'feature/api-client', 'fix/log-parse'];

const COMMIT_MSGS = [
  'fix: 로그 파싱 NULL 체크 추가',
  'feat: 세션 시각 자동 계산',
  'refactor: api.c 응답 처리 분리',
  'docs: README 테스트 방법 추가',
  'test: 날짜 유틸 단위 테스트',
];

// 오늘 날짜에만 09~15시 로그를 결정적으로 생성. 다른 날짜는 빈 배열(빈 상태 검증용).
export function mockLogsFor(teamId: number, date: string, onlyUserId?: number): ApiLog[] {
  if (date !== todayStr()) return [];

  const logs: ApiLog[] = [];
  for (const member of mockTeam.members) {
    const m = member.user_id;
    if (onlyUserId !== undefined && m !== onlyUserId) continue;

    for (let h = 9; h <= 15; h++) {
      if ((h + m * 2) % 3 === 0) continue; // 멤버당 4~5개 로그

      const memTotalGb = m % 2 === 0 ? 8 : 16;
      const memUsedGb =
        Math.round((((h * 1.3 + m * 2.1) % (memTotalGb - 2)) + 1.5) * 10) / 10;
      const hasCommits = (h + m) % 4 !== 0;
      const commitCount = 1 + ((h + m) % 3);

      logs.push({
        logId: m * 100 + h,
        userId: m,
        teamId,
        session: `${date}T${pad2(h)}:00`,
        cpuUsage: ((h * 7 + m * 13) % 91) + 5,
        memUsedGb,
        memTotalGb,
        branch: (h + m) % 5 === 0 ? null : BRANCHES[(h + m) % BRANCHES.length],
        comment: COMMENTS[(h + m) % COMMENTS.length],
        createdAt: `${date}T${pad2(h)}:${pad2((h * m) % 60)}:${pad2((h + m * 7) % 60)}`,
        commits: hasCommits
          ? Array.from(
              { length: commitCount },
              (_, i) => COMMIT_MSGS[(h + m + i) % COMMIT_MSGS.length],
            )
          : [],
      });
    }
  }
  return logs;
}
