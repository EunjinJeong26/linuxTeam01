export function pad2(n: number): string {
  return String(n).padStart(2, '0');
}

export function todayStr(): string {
  const d = new Date();
  return `${d.getFullYear()}-${pad2(d.getMonth() + 1)}-${pad2(d.getDate())}`;
}

export function addDays(date: string, delta: number): string {
  const d = new Date(`${date}T00:00:00`);
  d.setDate(d.getDate() + delta);
  return `${d.getFullYear()}-${pad2(d.getMonth() + 1)}-${pad2(d.getDate())}`;
}

export function splitSession(session: string): { date: string; hour: number } {
  return { date: session.slice(0, 10), hour: Number(session.slice(11, 13)) };
}

export function timeOf(createdAt: string): string {
  return createdAt.slice(11, 19);
}

export function range(from: number, to: number): number[] {
  return Array.from({ length: to - from + 1 }, (_, i) => from + i);
}

// 로그가 존재하는 최소~최대 시각 ±1시간. 로그 없으면 09~18시.
export function displayHours(hours: number[]): number[] {
  if (hours.length === 0) return range(9, 18);
  const min = Math.max(0, Math.min(...hours) - 1);
  const max = Math.min(23, Math.max(...hours) + 1);
  return range(min, max);
}
