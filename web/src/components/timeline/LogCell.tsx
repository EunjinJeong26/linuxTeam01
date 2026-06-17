import type { Log } from '../../types';
import { pad2 } from '../../utils/date';

interface LogCellProps {
  log: Log | undefined;
  selected: boolean;
  onSelect: (log: Log) => void;
}

export function LogCell({ log, selected, onSelect }: LogCellProps) {
  if (!log) {
    return (
      <div
        aria-hidden="true"
        className="h-8 rounded border border-dashed border-border bg-raised/40"
      />
    );
  }

  return (
    <button
      type="button"
      title={log.comment.split('\n')[0]}
      aria-label={`${log.username} ${pad2(log.sessionHour)}시 로그`}
      aria-pressed={selected}
      onClick={() => onSelect(log)}
      className={`h-8 w-full rounded bg-accent transition-colors hover:bg-accent/80 ${
        selected ? 'ring-2 ring-inset ring-text-primary' : ''
      }`}
    />
  );
}
