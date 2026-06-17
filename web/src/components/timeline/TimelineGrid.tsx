import { Fragment } from 'react';
import type { Log, Member } from '../../types';
import { HourAxis } from './HourAxis';
import { LogCell } from './LogCell';

interface TimelineGridProps {
  members: Member[];
  logs: Log[];
  hours: number[];
  selectedLogId: number | null;
  onSelect: (log: Log) => void;
}

export function TimelineGrid({
  members,
  logs,
  hours,
  selectedLogId,
  onSelect,
}: TimelineGridProps) {
  const byCell = new Map<string, Log>();
  for (const log of logs) {
    byCell.set(`${log.userId}-${log.sessionHour}`, log);
  }

  return (
    <div className="overflow-x-auto rounded border border-border bg-surface p-3">
      <div
        className="grid gap-1"
        style={{
          gridTemplateColumns: `72px repeat(${hours.length}, minmax(20px, 1fr))`,
          minWidth: 72 + hours.length * 26,
        }}
      >
        <HourAxis hours={hours} />
        {members.map((member) => (
          <Fragment key={member.userId}>
            <div className="flex items-center truncate pr-2 font-mono text-xs text-text-secondary">
              {member.username}
            </div>
            {hours.map((h) => {
              const log = byCell.get(`${member.userId}-${h}`);
              return (
                <LogCell
                  key={h}
                  log={log}
                  selected={log !== undefined && log.logId === selectedLogId}
                  onSelect={onSelect}
                />
              );
            })}
          </Fragment>
        ))}
      </div>
    </div>
  );
}
