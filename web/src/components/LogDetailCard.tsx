import type { Log } from '../types';
import { pad2, timeOf } from '../utils/date';
import { Badge } from './ui/Badge';

export function LogDetailCard({ log }: { log: Log | null }) {
  if (!log) {
    return (
      <div className="rounded border border-border bg-surface p-6 text-center text-text-muted">
        셀을 선택하면 상세 내용이 표시됩니다
      </div>
    );
  }

  return (
    <article className="rounded border border-border bg-surface p-4">
      <header className="mb-3 flex items-baseline justify-between gap-2">
        <span className="font-mono font-semibold">
          {log.username} · {pad2(log.sessionHour)}:00
        </span>
        <span className="font-mono text-xs text-text-muted">
          posted {timeOf(log.createdAt)}
        </span>
      </header>
      <p className="mb-3">{log.comment}</p>
      <div className="flex flex-wrap gap-1.5">
        {log.branch !== null && <Badge variant="git">{log.branch}</Badge>}
        {log.commitMessages.map((msg, i) => (
          <Badge key={i} variant="git">
            {msg}
          </Badge>
        ))}
        <Badge variant="sys">CPU {log.cpuUsage}%</Badge>
        <Badge variant="sys">
          MEM {log.memPercent}% ({log.memUsedGb.toFixed(1)}/{log.memTotalGb.toFixed(1)} GB)
        </Badge>
      </div>
    </article>
  );
}
