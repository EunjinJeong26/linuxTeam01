import { pad2 } from '../../utils/date';

export function HourAxis({ hours }: { hours: number[] }) {
  return (
    <>
      <div aria-hidden="true" />
      {hours.map((h) => (
        <div
          key={h}
          className="pb-1 text-center font-mono text-[10px] text-text-muted"
        >
          {pad2(h)}
        </div>
      ))}
    </>
  );
}
