# Devlog 웹 프론트엔드 명세서

> 본 문서는 Devlog 프로젝트의 React 웹 클라이언트(Phase 3) 구현 명세입니다.
> 구현 시 본 문서를 단일 기준으로 삼고, 문서와 충돌하는 판단이 필요하면 임의로 결정하지 말고 질문할 것.
> 상위 기획은 `PLAN.md` 참조.

**작성일:** 2026-06-12
**작업 위치:** 레포지토리의 `web/` 디렉터리
**작업 인원:** 1인 (프론트엔드 단독)

---

## 1. 개요

### 1.1 제품 컨텍스트

Devlog는 소규모 개발자 그룹(최대 6명)용 상태 공유 SNS다. 사용자는 Linux CLI 클라이언트에서 1시간 단위로 로그를 게시하며, 로그에는 자동 수집된 CPU/메모리 사용률, 최근 git 커밋, 짧은 코멘트(최대 150자)가 담긴다. 백엔드는 Spring Boot + SQLite로 구축되어 있다.

### 1.2 웹 클라이언트의 역할: **읽기 전용 뷰어**

웹은 CLI가 제공하지 못하는 **시각화**를 담당한다. 핵심 가치는 "하루 동안 팀 전체가 무엇을 했는지 한 화면에서 본다"이다.

**범위 (In Scope)**
- 로그인 / 회원가입 / 로그아웃
- 팀 타임라인 대시보드: 날짜별 팀 전체 로그 그리드 조회
- 내 로그 조회
- 팀 정보 조회 (팀명, 초대 코드, 멤버 목록)

**비범위 (Out of Scope) — 구현하지 말 것**
- 로그 작성/수정/삭제 (작성은 CLI 전용. 웹에서는 어떤 형태의 post UI도 만들지 않는다)
- 팀 생성/참가/탈퇴 (CLI 전용)
- 댓글, 반응, 채팅, 알림 설정 (제품 차원에서 미도입)
- 다크/라이트 테마 전환 (다크 단일 테마)

---

## 2. 기술 스택 (고정)

| 영역 | 선택 | 비고 |
|---|---|---|
| 빌드 | Vite | `npm create vite@latest` 기준 |
| 언어 | TypeScript (strict) | |
| UI | React 18+ 함수형 컴포넌트 + Hooks | 클래스 컴포넌트 금지 |
| 라우팅 | React Router v6+ | |
| 서버 상태 | TanStack Query v5 | 모든 API 호출은 Query/Mutation으로 래핑 |
| HTTP | axios | 인스턴스 1개, JWT 인터셉터 |
| 스타일 | Tailwind CSS | 디자인 토큰은 `tailwind.config` theme에 등록 |
| 전역 상태 | React Context (인증만) | Redux/Zustand 등 도입 금지 |
| 실시간 | 폴링 (TanStack Query `refetchInterval` 60s) | 서버는 STOMP/WebSocket이지만 본 클라이언트는 폴링으로 통일 (섹션 8 참조) |
| 테스트 | Vitest + React Testing Library | 핵심 유틸/그리드 로직 위주 |

추가 라이브러리 도입은 금지. 날짜 처리도 외부 라이브러리 없이 네이티브 `Date` + 직접 작성한 유틸로 처리한다 (다루는 형식이 `YYYY-MM-DD`와 0~23 정수뿐이라 충분하다).

---

## 3. 디자인 가이드

### 3.1 디자인 컨셉: "터미널의 연장"

웹은 CLI 제품의 확장이므로, 화면 자체가 잘 정돈된 터미널처럼 보여야 한다. 다크 단일 테마, 모노스페이스 중심, 장식 없는 평면 디자인. 그라디언트·글로우·네온·과한 그림자 금지.

시그니처 요소는 **타임라인 그리드**(섹션 4.2)다. 나머지 화면은 조용하고 절제되게 유지하고, 시각적 임팩트는 그리드에 집중시킨다.

### 3.2 디자인 토큰

```js
// tailwind.config — theme.extend.colors
colors: {
  bg:        '#0E1116',   // 페이지 배경
  surface:   '#161B22',   // 카드·패널
  raised:    '#1D232C',   // 셀 hover, 입력 필드
  border:    '#2A313B',   // 기본 보더
  text: {
    primary:   '#E6EDF3',
    secondary: '#9BA6B2',
    muted:     '#5C6670',
  },
  accent: {
    DEFAULT: '#3FB68B',   // 주 강조 (로그 게시 셀, 활성 상태, 브랜드)
    soft:    '#1E5C46',   // accent의 어두운 변형 (선택 배경 등)
  },
  git:   '#E9B259',       // git 커밋 정보 전용
  sys:   '#6CA9E8',       // CPU/MEM 정보 전용
  danger:'#E2645A',       // 에러
}
```

색의 의미는 고정한다: 초록=로그/활동, 앰버=git, 블루=시스템 지표. 이 매핑을 모든 화면에서 일관되게 유지한다.

### 3.3 타이포그래피

- **모노스페이스: `JetBrains Mono`** — 로고, 사용자명, 커밋 해시·메시지, 수치(CPU/MEM/시각), 초대 코드. 데이터는 전부 모노스페이스.
- **본문: `Pretendard Variable`** — 한국어 UI 레이블, 코멘트 본문, 안내 문구.
- 두 폰트 모두 CDN(jsdelivr) 또는 npm 패키지로 로드.
- 크기: 본문 14px, 보조 12px, 페이지 제목 18px. weight는 400/600 두 단계만.

### 3.4 레이아웃

- 상단 고정 헤더 1개: 좌측 `devlog / <팀명>` 로고, 우측 내비게이션(타임라인 · 내 로그 · 팀) + 사용자명 + 로그아웃.
- 본문 최대 폭 960px 중앙 정렬, 데스크톱 우선. 모바일(≥360px)에서 그리드는 가로 스크롤 허용.
- 모서리 radius 6px 통일. 보더는 1px.

---

## 4. 화면 명세

### 4.1 라우트 구조

| 경로 | 화면 | 접근 |
|---|---|---|
| `/login` | 로그인 | 비로그인 전용 (로그인 상태면 `/`로 리다이렉트) |
| `/register` | 회원가입 | 비로그인 전용 |
| `/` | 팀 타임라인 대시보드 | 인증 필요 |
| `/me` | 내 로그 | 인증 필요 |
| `/team` | 팀 정보 | 인증 필요 |

인증 필요 라우트는 가드 컴포넌트로 감싸고, 토큰이 없거나 만료(401)되면 `/login`으로 보낸다.

### 4.2 팀 타임라인 대시보드 (`/`) — 핵심 화면

**구성 (위→아래):** 날짜 내비게이션 → 타임라인 그리드 → 로그 상세 카드

**날짜 내비게이션**
- `◀ 2026-06-12 (오늘) ▶` 형태. 이전/다음 버튼 + 클릭 시 네이티브 date input.
- 미래 날짜로는 이동 불가. 기본값은 오늘.

**타임라인 그리드**
- 행 = 팀 멤버 (전원, 로그 없는 멤버도 표시), 열 = 시간 0~23.
- 단, 기본 표시 범위는 **로그가 존재하는 최소 시각 ~ 최대 시각**(여백 ±1시간)으로 좁히고, "전체 24시간 보기" 토글을 제공한다. 로그가 하나도 없으면 09~18시를 기본 표시.
- 셀 상태 3종:
  - **게시됨**: `accent` 채움. hover 시 코멘트 첫 줄 툴팁.
  - **빈 슬롯**: `raised` 배경 + `border` 점선. 클릭 불가.
  - **선택됨**: 게시 셀 클릭 시 밝은 테두리 강조.
- 게시 셀 클릭 → 아래 상세 카드에 해당 로그 표시. 같은 셀 재클릭 시 선택 해제.
- 키보드 접근: 게시 셀은 Tab 이동 + Enter 선택 가능해야 한다.

**로그 상세 카드**
- 헤더: `username · HH:00` (모노) + 우측에 `posted HH:MM:SS` (서버 `createdAt`의 시:분:초).
- 본문: 코멘트 (최대 150자, 서버가 항상 보장 → 줄임/null 처리 불필요).
- 하단 배지:
  - `git` 색 배지 1개: `branch` 표시 (예: `main`). branch가 null이면 배지 생략.
  - `git` 색 배지 N개: `commits[]`의 각 메시지를 개별 배지로 나열 (해시는 서버가 제공하지 않음). 빈 배열이면 생략.
  - `sys` 색 배지 2개: `CPU n%`, `MEM n% (n.n/n.n GB)` — MEM은 `memUsedGb/memTotalGb`로 퍼센트 계산 + 보조로 사용량/총량 GB 1자리 표기.
- CPU/MEM/코멘트는 서버 스키마상 NOT NULL이므로 null 처리 분기 불필요.
- 선택된 로그가 없을 때: "셀을 선택하면 상세 내용이 표시됩니다" 안내 문구만 muted로.

**상태별 화면**
- 로딩: 그리드 영역에 스켈레톤 (행 수 = 멤버 수 또는 4).
- 해당 날짜 로그 0건: 그리드는 빈 슬롯으로 채우고 상단에 "이 날짜에 게시된 로그가 없습니다" 안내.
- 에러: danger 색 안내 + "다시 시도" 버튼 (Query refetch).

### 4.3 내 로그 (`/me`)

- 동일한 날짜 내비게이션 재사용.
- 그리드 대신 **세로 피드**: 시간 오름차순으로 로그 상세 카드를 나열. 카드 컴포넌트는 4.2와 동일한 것 재사용.
- 빈 슬롯은 표시하지 않는다 (게시한 로그만).
- 0건일 때: "이 날짜에 작성한 로그가 없습니다."

### 4.4 팀 정보 (`/team`)

- 팀명만 표시 (서버에 팀 생성일 필드가 없음).
- 초대 코드: 모노스페이스 큰 글씨 + 복사 버튼 (`navigator.clipboard`, 성공 시 2초간 "복사됨" 표시).
- 멤버 목록 테이블: username(모노), role(`leader`/`member` 배지). 서버가 가입 시각을 제공하지 않으므로 joined_at 열 없음. `leader` 행을 맨 위에, 그 외는 username 알파벳 순.
- 하단에 muted 안내: "팀 생성·참가·탈퇴는 CLI(`devlog team ...`)에서 가능합니다."

### 4.5 로그인 / 회원가입 (`/login`, `/register`)

- 중앙 정렬 단일 카드 (최대 폭 360px). 상단에 `devlog` 로고(모노, accent).
- 로그인: username, password, 제출 버튼. 실패 시 카드 내 danger 문구 ("사용자명 또는 비밀번호가 올바르지 않습니다"). 성공 시 응답의 `token`/`user_id`/`username`을 모두 저장.
- 회원가입: username, password, password 확인. 클라이언트 검증(불일치 시 즉시 표시). **서버 register 응답에 토큰이 포함되지 않으므로 자동 로그인은 수행하지 않는다.** 성공 시 "회원가입이 완료되었습니다. 로그인해 주세요." 안내를 띄우고 `/login`으로 이동(username 프리필 선택).
- 폼 제출 중에는 버튼 비활성 + "처리 중..." 표시.

---

## 5. 데이터 모델 (TypeScript)

서버 응답(raw)과 프론트 내부 모델을 분리한다. raw → 내부 변환은 `src/api/mappers.ts`에서만 수행하고, 컴포넌트는 내부 모델만 사용한다. 정의 위치는 `src/types/index.ts`.

### 5.1 서버 응답 raw 타입

서버 DTO는 엔드포인트별로 명명 규칙이 **혼재**한다 (Auth/Team은 snake_case, Log/Notify는 camelCase). 매퍼에서 흡수.

```ts
// POST /auth/login 응답
export interface ApiLoginResponse {
  token: string;
  user_id: number;
  username: string;
}

// POST /auth/register 응답 (토큰 없음)
export interface ApiRegisterResponse {
  user_id: number;
  username: string;
}

// GET /teams/me 응답
export interface ApiMyTeamResponse {
  team_id: number;
  name: string;                 // teamName 아님
  invite_code: string;          // 영문 대문자+숫자 8자리
  members: ApiMemberInfo[];
}
export interface ApiMemberInfo {
  user_id: number;
  username: string;
  role: 'leader' | 'member';    // owner 아님
}

// GET /logs?... 응답 (배열)
export interface ApiLog {
  logId: number;
  userId: number;               // username은 없음
  teamId: number;
  session: string;              // 'YYYY-MM-DDTHH:00' 단일 문자열
  cpuUsage: number;             // NOT NULL
  memUsedGb: number;            // NOT NULL
  memTotalGb: number;           // NOT NULL
  branch: string | null;        // nullable
  comment: string;              // NOT NULL, ≤150자
  createdAt: string;            // 'YYYY-MM-DDTHH:mm:ss' (타임존 없음)
  commits: string[];            // 커밋 메시지 목록 (해시 없음)
}
```

### 5.2 프론트 내부 모델

```ts
export interface User {
  userId: number;
  username: string;
}

export interface Team {
  teamId: number;
  teamName: string;             // raw `name`을 리네임
  inviteCode: string;
  // 서버에 createdAt 필드 없음 → 모델에서도 제외
}

export interface Member {
  userId: number;
  username: string;
  role: 'leader' | 'member';
  // 서버에 joinedAt 없음 → 모델에서도 제외
}

export interface Log {
  logId: number;
  userId: number;
  username: string;             // 멤버 맵으로 join하여 채움
  teamId: number;
  sessionDate: string;          // session.slice(0, 10)
  sessionHour: number;          // parseInt(session.slice(11, 13))
  cpuUsage: number;             // %
  memPercent: number;           // round(memUsedGb / memTotalGb * 100)
  memUsedGb: number;            // 보조 표기용
  memTotalGb: number;           // 보조 표기용
  branch: string | null;
  commitMessages: string[];     // raw `commits`
  comment: string;
  createdAt: string;            // ISO local
}
```

### 5.3 username 결합 규칙

`ApiLog`에는 username이 없다. 화면 진입 시 `GET /teams/me`로 받은 `members` 배열로 `Map<userId, username>`을 만들어 두고, 로그 매핑 단계에서 join한다. 멤버 목록에 없는 `userId`(탈퇴 등)는 ``unknown(${userId})``로 표시한다.

---

## 6. API 연동

### 6.1 공통

- Base URL: `import.meta.env.VITE_API_BASE_URL`, 기본 `http://localhost:8080`. **서버는 `/api` prefix를 사용하지 않는다** — 경로는 `/auth/...`, `/teams/...`, `/logs`를 그대로 호출한다.
- 개발 중 CORS 회피: 서버에 HTTP CORS 설정이 없으므로 (`SecurityConfig`에 cors 설정 없음, WebSocket만 `*` 허용), 개발은 반드시 `vite.config.ts`의 `server.proxy`로 `/auth`, `/teams`, `/logs`를 서버 포트(8080)에 프록시한다. **배포 환경에서는 서버측 CORS 추가가 선행되어야 한다 (서버팀에 요청 필요).**
- 인증:
  - 로그인 성공 시 `localStorage`에 `devlog_token`(JWT), `devlog_user_id`(number), `devlog_username`(string)을 저장한다. 서버에 whoami(`/auth/me`) 엔드포인트가 없으므로 이 3개를 페이지 새로고침 후 신원 복구의 단일 출처로 사용한다.
  - axios 요청 인터셉터로 `Authorization: Bearer <token>` 첨부. 응답 인터셉터에서 401 수신 시 위 3개를 모두 삭제 후 `/login`으로 리다이렉트.
  - JWT 만료는 24시간 (서버 `JwtUtil.EXPIRATION_TIME` 기준).
- register 응답에는 토큰이 포함되지 않는다 → 회원가입 직후 자동 로그인 호출 금지 (섹션 4.5 참조).

### 6.2 엔드포인트 (서버 Controller 기준 확정)

| 기능 | 메서드/경로 | 요청 | 응답 |
|---|---|---|---|
| 회원가입 | `POST /auth/register` | `{ username, password }` | `{ user_id, username }` (201, 토큰 없음) |
| 로그인 | `POST /auth/login` | `{ username, password }` | `{ token, user_id, username }` (200) |
| 팀 정보 | `GET /teams/me` | — | `ApiMyTeamResponse` (`{ team_id, name, invite_code, members[] }`) |
| 팀 로그 조회 | `GET /logs?teamId={id}&date=YYYY-MM-DD&my=false` | — | `ApiLog[]` |
| 내 로그 조회 | `GET /logs?teamId={id}&date=YYYY-MM-DD&my=true` | — | `ApiLog[]` |

추가 사항:
- `/logs`는 **`teamId` 쿼리 파라미터가 필수**다. 프론트는 첫 진입 시 `GET /teams/me`를 먼저 호출해 `team_id`를 얻고, 이후 로그 조회 훅에 주입한다 (섹션 7).
- 서버에는 `/users/me/notify` GET/PATCH도 존재하지만 웹은 읽기 전용 뷰어이므로 호출하지 않는다.
- 401 외의 4xx/5xx 응답은 본문이 일관된 포맷이 아닐 수 있으므로(`ResponseStatusException`의 reason 또는 빈 본문) 화면용 메시지는 상태 코드 기준으로 변환한다.

### 6.3 Mock 모드 (서버 없이 개발)

서버 가동 없이도 전 화면을 개발·시연할 수 있도록 mock 레이어를 만든다.

- `VITE_USE_MOCK=true`일 때 axios 대신 `src/api/mock/`의 더미 구현을 사용 (API 모듈에서 분기).
- 더미 데이터: 멤버 4명(`jin`(leader), `minseo`, `hyun`, `sora`), 오늘 날짜 기준 09~15시에 멤버당 3~6개 로그. `branch`가 null인 케이스와 `commits`가 빈 배열인 케이스를 섞어 배지 렌더링 분기를 검증한다. CPU/MEM은 NOT NULL이므로 항상 값 채움.
- 네트워크 지연 흉내: 300ms `setTimeout`.

구현 1단계는 mock 모드로 진행하고, 서버 가동 후 실 API로 전환한다.

---

## 7. 디렉터리 구조

```text
web/
├── src/
│   ├── api/
│   │   ├── client.ts        # axios 인스턴스 + 인터셉터 (Bearer, 401 처리)
│   │   ├── mappers.ts       # ApiLog → Log, ApiMyTeamResponse → Team/Member[]
│   │   ├── auth.ts
│   │   ├── logs.ts          # getTeamLogs(teamId, date), getMyLogs(teamId, date)
│   │   ├── teams.ts         # getMyTeam()
│   │   └── mock/            # VITE_USE_MOCK 시 사용되는 더미 구현
│   ├── types/index.ts       # Api* (raw) + 내부 모델
│   ├── hooks/
│   │   ├── useAuth.ts       # AuthContext 소비
│   │   ├── useTeam.ts       # useQuery(['team'], …) — teamId/members 공급
│   │   ├── useTeamLogs.ts   # useQuery(['logs', 'team', teamId, date], …, enabled: !!teamId)
│   │   └── useMyLogs.ts     # useQuery(['logs', 'my',   teamId, date], …, enabled: !!teamId)
│   ├── contexts/AuthContext.tsx
│   ├── components/
│   │   ├── layout/Header.tsx
│   │   ├── timeline/TimelineGrid.tsx
│   │   ├── timeline/LogCell.tsx
│   │   ├── timeline/HourAxis.tsx
│   │   ├── LogDetailCard.tsx
│   │   ├── DateNavigator.tsx
│   │   └── ui/              # Badge, Skeleton, ErrorBox 등 범용
│   ├── pages/
│   │   ├── LoginPage.tsx
│   │   ├── RegisterPage.tsx
│   │   ├── DashboardPage.tsx
│   │   ├── MyLogsPage.tsx
│   │   └── TeamPage.tsx
│   ├── utils/date.ts        # session 문자열 split, 시각 범위 계산
│   ├── App.tsx              # 라우터 + 가드
│   └── main.tsx
├── .env.development
├── tailwind.config.ts
└── vite.config.ts
```

**훅 의존 순서:** `useTeam` → (`useTeamLogs` | `useMyLogs`). 로그 훅은 `enabled: !!teamId`로 게이팅하고, 매퍼 단계에서 `useTeam`이 반환한 `members`로 username을 join한다. 토큰 만료/팀 미소속(404) 시의 분기는 각각 401 인터셉터 / TeamPage 안내로 처리한다.

---

## 8. 실시간 갱신 — 폴링 채택

서버는 STOMP over WebSocket(`/ws`, `/topic/team/{teamId}`)을 제공하지만, 클라이언트는 다음 이유로 **폴링만 사용한다**:

- STOMP 클라이언트 라이브러리(`@stomp/stompjs` 등)는 섹션 2의 "추가 라이브러리 금지" 규칙과 충돌.
- 시간 해상도가 1시간 단위라 60초 폴링으로도 사용자 경험상 차이가 없다.

구현:
- TanStack Query의 `refetchInterval: 60_000`을 `useTeamLogs`/`useMyLogs`에 적용.
- 보고 있는 날짜가 "오늘"일 때만 폴링하고, 과거 날짜에서는 `refetchInterval`을 `false`로 두어 불필요한 요청을 막는다 (날짜 변동에 따라 동적으로 옵션 결정).
- `refetchOnWindowFocus: true`도 함께 설정해, 탭 전환 후 복귀 시 즉시 최신화.
- 헤더에 별도의 연결 상태 점 표시는 두지 않는다. 폴링 실패는 Query의 에러 상태로만 표면화하고, 카드/그리드 안에서 danger 메시지로 처리.

---

## 9. 품질 기준

- TypeScript strict 모드에서 에러 0.
- 모든 화면에 로딩/빈 상태/에러 상태 구현. "흰 화면" 또는 무한 스피너 금지.
- 키보드만으로 로그인 → 셀 선택 → 상세 확인 동선이 가능할 것. 포커스 링 가시화.
- 콘솔 에러/경고 0 (React key 경고 포함).
- 그리드 24열 표시 시에도 데스크톱(1280px)에서 가로 스크롤 없이 렌더링, 모바일에서는 가로 스크롤 허용.
- 수치 표시는 반올림: CPU/MEM은 정수 % 표기.

---

## 10. 구현 순서 (마일스톤)

각 단계가 끝날 때마다 동작 확인 후 다음 단계로 진행한다.

1. **M1 — 골격**: Vite + TS + Tailwind + Router 셋업, 디자인 토큰 등록, 헤더/레이아웃, 라우트 골격, 인증 가드 (mock 토큰).
2. **M2 — 타임라인 그리드 (mock)**: 더미 데이터로 DashboardPage 완성. 그리드, 셀 상태 3종, 상세 카드, 날짜 내비게이션, 시각 범위 축소/전체 토글.
3. **M3 — 나머지 화면 (mock)**: 내 로그, 팀 정보, 로그인/회원가입 폼.
4. **M4 — 실 API 연동**: mock 분기 유지한 채 실 API 연결 (6.2 표 기준), `useTeam` → `useTeamLogs`/`useMyLogs` 의존 배선, JWT 플로우, 401 처리, 매퍼·username join 검증.
5. **M5 — 폴링/마감**: `refetchInterval` 적용(오늘만), 품질 기준(섹션 9) 전수 점검, 반응형 마무리.

---

## 11. 확인 필요 사항

### 11.1 해소된 항목 (서버 코드 대조 완료, 2026-06-12)

| # | 항목 | 결론 |
|---|---|---|
| 1 | REST 경로·응답 JSON 형식 | `/api` prefix 없음. Auth/Team은 snake_case, Log/Notify는 camelCase 혼재 → 매퍼에서 흡수 (섹션 6.2, 5) |
| 2 | 서버 포트 | `application.yml`에 미지정 → Spring 기본 **8080** |
| 4 | WebSocket | STOMP 존재 (`/ws`, `/topic/team/{teamId}`). 그러나 라이브러리 도입 비용 회피를 위해 **폴링 채택** (섹션 8) |
| 5 | JWT 만료 시간 | **24시간 고정** (`JwtUtil.EXPIRATION_TIME`) |

### 11.2 미해소 / 서버팀 의존

| # | 항목 | 영향 |
|---|---|---|
| 3 | HTTP CORS 설정 | 서버 `SecurityConfig`에 CORS 미설정. 개발은 Vite proxy로 우회 가능하나 **배포 전 서버팀에 CORS 추가 요청 필수** |
| 6 | 에러 응답 본문 포맷 | 서버가 `ResponseStatusException`을 그대로 던지므로 본문이 일관되지 않음 → 사용자 메시지는 상태 코드 기준으로 변환 (`401/403/404/409/5xx`) |

### 11.3 운영 메모

- register 응답에 토큰이 없음 → 회원가입 직후 자동 로그인 금지, `/login`으로만 이동 (섹션 4.5).
- 로그 응답에는 username이 없음 → 멤버 맵으로 join 필수 (섹션 5.3).
- 팀 미소속 사용자가 로그인하면 `GET /teams/me`가 404 → TeamPage·Dashboard에서 "CLI에서 팀에 참가/생성하세요" 안내로 처리.
