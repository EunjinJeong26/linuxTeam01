import { addDays, displayHours, pad2, range, splitSession, timeOf } from './date';

describe('splitSession', () => {
  it('세션 문자열을 날짜와 시각으로 분리한다', () => {
    expect(splitSession('2026-06-12T09:00')).toEqual({ date: '2026-06-12', hour: 9 });
    expect(splitSession('2026-01-01T00:00')).toEqual({ date: '2026-01-01', hour: 0 });
    expect(splitSession('2026-12-31T23:00')).toEqual({ date: '2026-12-31', hour: 23 });
  });
});

describe('addDays', () => {
  it('월 경계를 넘는다', () => {
    expect(addDays('2026-06-01', -1)).toBe('2026-05-31');
    expect(addDays('2026-06-30', 1)).toBe('2026-07-01');
  });
  it('연 경계를 넘는다', () => {
    expect(addDays('2026-12-31', 1)).toBe('2027-01-01');
    expect(addDays('2026-01-01', -1)).toBe('2025-12-31');
  });
});

describe('displayHours', () => {
  it('로그가 없으면 09~18시를 반환한다', () => {
    expect(displayHours([])).toEqual(range(9, 18));
  });
  it('최소~최대 ±1시간으로 좁힌다', () => {
    expect(displayHours([9, 12, 15])).toEqual(range(8, 16));
  });
  it('0시와 23시 경계를 벗어나지 않는다', () => {
    expect(displayHours([0, 23])).toEqual(range(0, 23));
    expect(displayHours([0])).toEqual(range(0, 1));
    expect(displayHours([23])).toEqual(range(22, 23));
  });
});

describe('timeOf / pad2', () => {
  it('createdAt에서 HH:MM:SS를 추출한다', () => {
    expect(timeOf('2026-06-12T09:07:23')).toBe('09:07:23');
  });
  it('한 자리 수를 0으로 채운다', () => {
    expect(pad2(5)).toBe('05');
    expect(pad2(15)).toBe('15');
  });
});
