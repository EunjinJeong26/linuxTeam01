import { useMemo } from 'react';
import { useQuery } from '@tanstack/react-query';
import { getMyLogs } from '../api/logs';
import { mapLog } from '../api/mappers';
import { todayStr } from '../utils/date';
import type { Log, Member } from '../types';

export function useMyLogs(
  teamId: number | undefined,
  date: string,
  members: Member[] | undefined,
) {
  const query = useQuery({
    queryKey: ['logs', 'my', teamId, date],
    queryFn: () => getMyLogs(teamId as number, date),
    enabled: teamId !== undefined,
    refetchInterval: date === todayStr() ? 60_000 : false,
  });

  const logs = useMemo<Log[] | undefined>(() => {
    if (!query.data) return undefined;
    const nameMap = new Map<number, string>(
      (members ?? []).map((m) => [m.userId, m.username]),
    );
    return query.data
      .map((raw) => mapLog(raw, nameMap))
      .sort((a, b) => a.sessionHour - b.sessionHour);
  }, [query.data, members]);

  return {
    logs,
    isError: query.isError,
    refetch: query.refetch,
  };
}
