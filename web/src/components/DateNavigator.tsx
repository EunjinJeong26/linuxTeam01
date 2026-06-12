import { useRef } from 'react';
import { addDays, todayStr } from '../utils/date';

interface DateNavigatorProps {
  date: string;
  onChange: (date: string) => void;
}

export function DateNavigator({ date, onChange }: DateNavigatorProps) {
  const inputRef = useRef<HTMLInputElement>(null);
  const today = todayStr();

  function openPicker() {
    const el = inputRef.current;
    if (!el) return;
    try {
      el.showPicker();
    } catch {
      el.focus();
    }
  }

  return (
    <div className="relative flex items-center justify-center gap-2">
      <button
        type="button"
        aria-label="이전 날짜"
        onClick={() => onChange(addDays(date, -1))}
        className="rounded border border-border px-2 py-1 text-text-secondary hover:bg-raised hover:text-text-primary"
      >
        ◀
      </button>
      <button
        type="button"
        onClick={openPicker}
        className="rounded border border-border px-3 py-1 font-mono hover:bg-raised"
      >
        {date}
        {date === today && <span className="text-text-secondary"> (오늘)</span>}
      </button>
      <input
        ref={inputRef}
        type="date"
        value={date}
        max={today}
        onChange={(e) => e.target.value && onChange(e.target.value)}
        tabIndex={-1}
        aria-hidden="true"
        className="absolute h-0 w-0 opacity-0"
      />
      <button
        type="button"
        aria-label="다음 날짜"
        disabled={date >= today}
        onClick={() => onChange(addDays(date, 1))}
        className="rounded border border-border px-2 py-1 text-text-secondary hover:bg-raised hover:text-text-primary disabled:cursor-not-allowed disabled:opacity-40 disabled:hover:bg-transparent"
      >
        ▶
      </button>
    </div>
  );
}
