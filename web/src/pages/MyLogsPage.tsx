import { useState } from 'react';
import { useTeam } from '../hooks/useTeam';
import { useMyLogs } from '../hooks/useMyLogs';
import { DateNavigator } from '../components/DateNavigator';
import { LogDetailCard } from '../components/LogDetailCard';
import { ErrorBox } from '../components/ui/ErrorBox';
import { Skeleton } from '../components/ui/Skeleton';
import { todayStr } from '../utils/date';

export function MyLogsPage() {
  const [date, setDate] = useState(todayStr);

  const team = useTeam();
  const logsQuery = useMyLogs(team.team?.teamId, date, team.members);
  const { logs } = logsQuery;

  const isError = team.isError || logsQuery.isError;
  const isLoading = !isError && !team.noTeam && logs === undefined;

  return (
    <section className="flex flex-col gap-4">
      <h2 className="text-lg font-semibold">내 로그</h2>

      <DateNavigator date={date} onChange={setDate} />

      {team.noTeam ? (
        <p className="rounded border border-border bg-surface p-6 text-center text-text-muted">
          소속된 팀이 없습니다. CLI(<code className="font-mono">devlog team create/join</code>)로
          팀에 참가한 후 이용할 수 있습니다.
        </p>
      ) : isError ? (
        <ErrorBox
          message="데이터를 불러오지 못했습니다."
          onRetry={() => {
            void team.refetch();
            void logsQuery.refetch();
          }}
        />
      ) : isLoading ? (
        <div className="flex flex-col gap-3">
          {Array.from({ length: 3 }, (_, i) => (
            <Skeleton key={i} className="h-28 w-full" />
          ))}
        </div>
      ) : logs !== undefined && logs.length === 0 ? (
        <p className="rounded border border-border bg-surface p-6 text-center text-text-muted">
          이 날짜에 작성한 로그가 없습니다.
        </p>
      ) : (
        <div className="flex flex-col gap-3">
          {(logs ?? []).map((log) => (
            <LogDetailCard key={log.logId} log={log} />
          ))}
        </div>
      )}
    </section>
  );
}
