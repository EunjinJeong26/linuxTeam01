import { useState } from 'react';
import { useTeam } from '../hooks/useTeam';
import { useTeamLogs } from '../hooks/useTeamLogs';
import { DateNavigator } from '../components/DateNavigator';
import { TimelineGrid } from '../components/timeline/TimelineGrid';
import { LogDetailCard } from '../components/LogDetailCard';
import { ErrorBox } from '../components/ui/ErrorBox';
import { Skeleton } from '../components/ui/Skeleton';
import { displayHours, range, todayStr } from '../utils/date';
import type { Log } from '../types';

function GridSkeleton({ rows }: { rows: number }) {
  return (
    <div className="flex flex-col gap-2 rounded border border-border bg-surface p-3">
      {Array.from({ length: rows }, (_, i) => (
        <Skeleton key={i} className="h-8 w-full" />
      ))}
    </div>
  );
}

export function DashboardPage() {
  const [date, setDate] = useState(todayStr);
  const [selectedLogId, setSelectedLogId] = useState<number | null>(null);
  const [showFull, setShowFull] = useState(false);

  const team = useTeam();
  const logsQuery = useTeamLogs(team.team?.teamId, date, team.members);
  const { logs } = logsQuery;

  function changeDate(next: string) {
    setDate(next);
    setSelectedLogId(null);
  }

  function toggleSelect(log: Log) {
    setSelectedLogId((prev) => (prev === log.logId ? null : log.logId));
  }

  const isError = team.isError || logsQuery.isError;
  const isLoading =
    !isError && !team.noTeam && (team.members === undefined || logs === undefined);
  const hours = showFull
    ? range(0, 23)
    : displayHours((logs ?? []).map((l) => l.sessionHour));
  const selectedLog = logs?.find((l) => l.logId === selectedLogId) ?? null;

  return (
    <section className="flex flex-col gap-4">
      <div className="flex items-center justify-between gap-2">
        <h2 className="text-lg font-semibold">팀 타임라인</h2>
        <label className="flex cursor-pointer items-center gap-1.5 text-xs text-text-secondary">
          <input
            type="checkbox"
            checked={showFull}
            onChange={(e) => setShowFull(e.target.checked)}
            className="accent-[#3FB68B]"
          />
          전체 24시간 보기
        </label>
      </div>

      <DateNavigator date={date} onChange={changeDate} />

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
        <GridSkeleton rows={team.members?.length ?? 4} />
      ) : (
        <>
          {logs !== undefined && logs.length === 0 && (
            <p className="text-center text-sm text-text-muted">
              이 날짜에 게시된 로그가 없습니다
            </p>
          )}
          <TimelineGrid
            members={team.members ?? []}
            logs={logs ?? []}
            hours={hours}
            selectedLogId={selectedLogId}
            onSelect={toggleSelect}
          />
          <LogDetailCard log={selectedLog} />
        </>
      )}
    </section>
  );
}
