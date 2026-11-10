# Test plan & quy trình Git — Project 1 (xv6)

Nguồn yêu cầu: `Project_1.pdf` (gọi tắt là "brief"). Các kết quả mong đợi bên dưới lấy từ brief
hoặc từ hành vi mã nguồn xv6 đã kiểm tra trong repo này; chỗ nào phụ thuộc vào thiết kế mà brief
không quy định thì ghi **DECISION NEEDED** (tổng hợp ở mục 5).

| Bài | Phụ trách | Reviewer | Branch gốc theo brief |
|-----|-----------|----------|-----------------------|
| xargs | Trung Kiên | Diễm Thúy | `util` |
| tree | Diễm Thúy | Trung Kiên | `util` |
| tracing | Khôi Nguyên | Đăng Khoa | `syscall` |
| procinfo | Đăng Khoa | Khôi Nguyên | `syscall` |

Quy ước ID: `XA` = xargs, `TR` = tree, `TC` = tracing, `PI` = procinfo.
Cột *Result* để trống, điền `PASS` / `FAIL` / `N/A` sau khi chạy thật trong QEMU.

---

## 1. Bảng test

### Ghi chú chung

- Chạy từ file system sạch: `make clean && make qemu`. File system được giữ giữa các lần chạy
  QEMU (brief, 4.1.1), nên PID và nội dung thư mục chỉ khớp với bảng khi chạy trên máy vừa boot.
- PID trong output phụ thuộc vào số lệnh đã chạy trước đó. Trên máy vừa boot thì `init` = 1,
  `sh` = 2, lệnh đầu tiên = 3. Brief cũng nói PID có thể khác.
- Giới hạn của `sh` trong xv6 (`user/sh.c`): mỗi lệnh tối đa **9 từ** (`MAXARGS 10`, nếu vượt thì
  `panic("too many args")`) và mỗi dòng lệnh tối đa **99 ký tự** (`buf[100]`). Vì vậy một số test
  phải tạo input qua file trung gian.
- `echo` không có tham số thì **không in gì, kể cả `\n`** (`user/echo.c`), nên `echo > f` tạo ra
  file rỗng.
- Hệ thống không có lệnh `printf`. `echo` luôn kết thúc bằng `\n`, và `README` (file duy nhất được
  đưa vào `fs.img` ngoài các chương trình) cũng kết thúc bằng `\n`.

### 1.1 xargs (`user/xargs.c`)

`MAXARG = 32` (`kernel/param.h`). Mảng `argv` cần một ô cho `0` ở cuối, nên lệnh được `exec` có tối
đa **31 tham số** (MAXARG-1). `sys_exec` cũng trả lỗi khi `argv` có từ 32 phần tử trở lên
(`kernel/sysfile.c`).

| ID | Purpose | Command(s) typed in the xv6 shell | Expected output | Requirement in brief | Evidence file | Result |
|----|---------|-----------------------------------|-----------------|----------------------|---------------|--------|
| XA-01 | Ví dụ 1 của brief | `echo hello too \| xargs echo bye` | `bye hello too` | 4.1.1 ví dụ 1 | `docs/test-evidence/xargs/XA-01.png` | |
| XA-02 | Ví dụ 2 của brief (có `-n 1`) | `(echo 1 ; echo 2) \| xargs -n 1 echo` | `1` rồi `2` trên 2 dòng. **DECISION NEEDED**: xargs có phải nhận cờ `-n 1` không? Nếu không xử lý, chương trình sẽ `exec` lệnh `-n` và thất bại. | 4.1.1 ví dụ 2 | `docs/test-evidence/xargs/XA-02.png` | |
| XA-03 | Nhiều dòng, không dùng `-n` (giống `grade-lab-util`) | `(echo 1 ; echo 2) \| xargs echo` | `1` rồi `2` trên 2 dòng (mỗi dòng chạy lệnh một lần) | 4.1.1 "runs the command for each line"; `grade-lab-util` test_xargs_multiline | `docs/test-evidence/xargs/XA-03.png` | |
| XA-04 | Nhiều dòng, giữ đúng thứ tự (cha phải `wait` con) | `(echo a ; echo b ; echo c) \| xargs echo line` | `line a`, `line b`, `line c` đúng thứ tự | 4.1.1 hint fork/exec/wait | `docs/test-evidence/xargs/XA-04.png` | |
| XA-05 | Tách dòng thành nhiều tham số | `echo d1 d2 \| xargs mkdir` rồi `ls` | Có 2 thư mục `d1` và `d2`, không phải một thư mục tên `d1 d2` | 4.1.1 "the additional arguments are "hello too"" | `docs/test-evidence/xargs/XA-05.png` | |
| XA-06 | Input rỗng | `echo \| xargs echo bye` | Không in gì, prompt `$` quay lại vì có 0 dòng nên lệnh chạy 0 lần. (GNU xargs sẽ in `bye`, nhưng brief chỉ nói "for each line".) | 4.1.1 | `docs/test-evidence/xargs/XA-06.png` | |
| XA-07 | Dòng trống | `grep ^$ README \| xargs echo X` (README có 8 dòng trống trên `syscall`, 9 trên `util`) | **DECISION NEEDED**: bỏ qua dòng trống (không in gì, giống UNIX) hay chạy `echo X` cho mỗi dòng trống (in 8 hoặc 9 dòng `X`) | không quy định | `docs/test-evidence/xargs/XA-07.png` | |
| XA-08 | Dòng cuối không có `\n` | Không tạo được bằng công cụ có sẵn trong xv6 (`echo` luôn thêm `\n`, README kết thúc bằng `\n`, không có `printf`). Cách test: tạo file trên host (`printf 'a\nb' > lastline`), tạm thêm vào lệnh `mkfs` trong `Makefile` **trên máy cá nhân, không commit**, rồi chạy `xargs echo X < lastline`. Nếu không làm được thì kiểm tra bằng review code. | **DECISION NEEDED**: có chạy lệnh cho phần dư `b` không (UNIX có chạy và in `X a`, `X b`)? | không quy định | `docs/test-evidence/xargs/XA-08.png` | |
| XA-09 | `exec` thất bại | `(echo a ; echo b) \| xargs nosuchcmd` | Không treo, không crash, prompt quay lại. Nội dung thông báo lỗi (ví dụ `xargs: exec nosuchcmd failed`) và việc tiếp tục hay dừng sau dòng lỗi: **DECISION NEEDED** | 4.1.1 hint fork/exec | `docs/test-evidence/xargs/XA-09.png` | |
| XA-10 | Thiếu lệnh | `echo hi \| xargs` | **DECISION NEEDED**: in usage rồi `exit(1)`, hay mặc định chạy `echo` như UNIX (in `hi`)? | không quy định | `docs/test-evidence/xargs/XA-10.png` | |
| XA-11 | Chuẩn bị input dài (mỗi bước có tối đa 9 từ và 99 ký tự) | `echo a b c d e f g h > f0`<br>`xargs echo 1 2 3 4 5 6 7 < f0 > f1`<br>`xargs echo 1 2 3 4 5 6 7 < f1 > f2`<br>`xargs echo 1 2 3 4 5 6 7 < f2 > f3`<br>`cat f3` | `cat f3` in một dòng 29 từ: `1 2 3 4 5 6 7 1 2 3 4 5 6 7 1 2 3 4 5 6 7 a b c d e f g h` | 4.1.1 hint MAXARG | `docs/test-evidence/xargs/XA-11.png` | |
| XA-12 | Biên đúng MAXARG-1 = 31 tham số | `xargs echo X < f3` (sau XA-11) | `X 1 2 3 4 5 6 7 1 2 3 4 5 6 7 1 2 3 4 5 6 7 a b c d e f g h` (`argv` có 31 phần tử: `echo` + `X` + 29 từ) | 4.1.1 hint MAXARG | `docs/test-evidence/xargs/XA-12.png` | |
| XA-13 | Vượt MAXARG-1 (32 và 33 tham số) | `xargs echo X Y < f3`<br>`xargs echo X Y Z < f3` | Không ghi tràn mảng `argv`, không crash, không treo, prompt quay lại. Cách xử lý (in lỗi và bỏ dòng, hay cắt bớt): **DECISION NEEDED** | 4.1.1 hint MAXARG | `docs/test-evidence/xargs/XA-13.png` | |
| XA-14 | Script chấm của repo | `sh < xargstest.sh` (chỉ có trên `util`) | 3 lần `hello` theo `grade-lab-util`. Lưu ý: script gọi `find`, mà `find` không thuộc brief và không có trong repo, nên hiện tại sẽ in `exec find failed`. | `grade-lab-util` test_xargs | `docs/test-evidence/xargs/XA-14.png` | |

Dọn dẹp sau test: `rm f0`, `rm f1`, `rm f2`, `rm f3`, hoặc `make clean`.

### 1.2 tree (`user/tree.c`)

Thụt lề: 2 khoảng trắng mỗi cấp (comment `// Two spaces per level` ở hint 3). Thứ tự in là thứ tự
entry trong thư mục, tức thứ tự tạo (xem ví dụ brief: `b`, `aa/`, `ab/`). Chương trình không sắp xếp.

| ID | Purpose | Command(s) typed in the xv6 shell | Expected output | Requirement in brief | Evidence file | Result |
|----|---------|-----------------------------------|-----------------|----------------------|---------------|--------|
| TR-01 | Ví dụ của brief | `mkdir a`<br>`echo > a/b`<br>`mkdir a/aa`<br>`echo > a/aa/b`<br>`echo > a/aa/c`<br>`mkdir a/ab`<br>`echo > a/ab/d`<br>`tree a` | <pre>a/<br>  b<br>  aa/<br>    b<br>    c<br>  ab/<br>    d</pre> | 4.1.2 Testing | `docs/test-evidence/tree/TR-01.png` | |
| TR-02 | Không có tham số | `tree` | Giống hệt `tree .` (TR-03) | 4.1.2 "If no argument provided, use current directory" | `docs/test-evidence/tree/TR-02.png` | |
| TR-03 | Thư mục hiện tại | `tree .` | Liệt kê đệ quy mọi entry của `/` (cwd của `sh` là `/`) theo thứ tự của `ls .`: `README`, các chương trình, `console` (thiết bị, không có `/`), và cây `a/` như TR-01. Không đệ quy vào `.` và `..`, không lặp vô hạn. Dòng gốc (`./` hay `.`?): **DECISION NEEDED** | 4.1.2 Additional test cases | `docs/test-evidence/tree/TR-03.png` | |
| TR-04 | Thư mục gốc | `tree /` | Nội dung giống TR-03. Dòng gốc (`/` hay `//`?): **DECISION NEEDED** | 4.1.2 Additional test cases | `docs/test-evidence/tree/TR-04.png` | |
| TR-05 | Lồng sâu hơn | (sau TR-01) `mkdir a/aa/aaa`<br>`echo > a/aa/aaa/file`<br>`tree a` | <pre>a/<br>  b<br>  aa/<br>    b<br>    c<br>    aaa/<br>      file<br>  ab/<br>    d</pre> | 4.1.2 Additional test cases | `docs/test-evidence/tree/TR-05.png` | |
| TR-06 | Thư mục không tồn tại | `tree nosuch` | In lỗi rồi thoát, prompt quay lại, không in cây. Nội dung thông báo **DECISION NEEDED** (đề xuất theo `ls.c`: `tree: cannot open nosuch`, ra fd 2) | 4.1.2 Error Handling | `docs/test-evidence/tree/TR-06.png` | |
| TR-07 | Tham số là file | `tree README` | **DECISION NEEDED**: in `README` giống cách `ls` xử lý file, hay báo lỗi "not a directory"? | brief chỉ nói `tree <directory>` | `docs/test-evidence/tree/TR-07.png` | |
| TR-08 | Thư mục rỗng | `mkdir e`<br>`tree e` | Chỉ một dòng gốc (`e/`) | 4.1.2 | `docs/test-evidence/tree/TR-08.png` | |
| TR-09 | Bỏ qua entry đã xoá (`inum == 0`, giống `ls.c`) | (sau TR-05) `rm a/ab/d`<br>`tree a` | Như TR-05 nhưng `ab/` không còn dòng con nào, không có dòng trống hay lỗi `stat` | 4.1.2 hint 1 "Look at user/ls.c" | `docs/test-evidence/tree/TR-09.png` | |
| TR-10 | Tham số có `/` ở cuối | `tree a/` | Cây giống TR-05/TR-09 (namei của xv6 bỏ qua `//` thừa). Dòng gốc thuộc cùng quyết định với TR-03 | 4.1.2 hint 6 path building | `docs/test-evidence/tree/TR-10.png` | |
| TR-11 | `stat` lỗi trên một entry | Không tạo được bằng công cụ có sẵn, nên kiểm tra bằng review code | Code in lỗi rồi `continue`, không `exit` | 4.1.2 Error Handling | review trong PR | |

### 1.3 tracing (`trace` syscall)

Số syscall (`kernel/syscall.h`): fork=1, read=5, exec=7, open=15, write=16, close=21.
Vì vậy `32 = 1<<SYS_read`, `2 = 1<<SYS_fork`, `32896 = exec|open`, `2097152 = 1<<SYS_close`.

Giá trị `read`: brief in `1023, 966, 70, 0`, nhưng README trong repo này khác README của MIT.
Mô phỏng vòng lặp `read` trong `user/grep.c` (`buf[1024]`) trên README thực tế cho kết quả
`1023, 971, 298, 0` (README trên `syscall`, 2292 byte) và `1023, 971, 409, 0` (README trên `util`,
2403 byte). `grade-lab-syscall` chỉ kiểm tra `\d+` nên cả hai đều đạt.

| ID | Purpose | Command(s) typed in the xv6 shell | Expected output | Requirement in brief | Evidence file | Result |
|----|---------|-----------------------------------|-----------------|----------------------|---------------|--------|
| TC-01 | Ví dụ 1: chỉ trace `read` | `trace 32 grep hello README` | <pre>3: syscall read -> 1023<br>3: syscall read -> 991<br>3: syscall read -> 324<br>3: syscall read -> 0</pre>(PID 3 khi chạy ngay sau boot. Brief in 966/70, khác do README.) | 4.2.1 ví dụ 1 | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |
| TC-02 | Ví dụ 2: trace mọi syscall | `trace 2147483647 grep hello README` | <pre>4: syscall trace -> 0<br>4: syscall exec -> 3<br>4: syscall open -> 3<br>4: syscall read -> 1023<br>4: syscall read -> 991<br>4: syscall read -> 324<br>4: syscall read -> 0<br>4: syscall close -> 0</pre>(`exec` trả về `argc` = 3, xem `kernel/exec.c`) | 4.2.1 ví dụ 2 | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |
| TC-03 | Ví dụ 3: process không được trace thì không in gì. Chạy **ngay sau** TC-01/TC-02 để chứng minh `sh` không bị ảnh hưởng | `grep hello README` | Không in gì (README không chứa `hello`) | 4.2.1 ví dụ 3, "should not affect other processes" | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |
| TC-04 | Ví dụ 4: con cháu kế thừa mask | `trace 2 usertests forkforkfork` | `usertests starting`, `test forkforkfork: ` lẫn với các dòng `N: syscall fork -> M` từ nhiều PID khác nhau (con, cháu…), có dòng `-> -1` khi hết slot process, cuối cùng là `OK`. Ngay sau boot: `3: syscall fork -> 4` (theo `grade-lab-syscall`) | 4.2.1 ví dụ 4, "any children that it subsequently forks" | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |
| TC-05 | Mask không trace gì | `trace 0 grep hello README` | Không có dòng `syscall` nào | 4.2.1 mask bits | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |
| TC-06 | Mask chỉ có bit không ứng với syscall nào (bit 0) | `trace 1 grep hello README` | Không có dòng `syscall` nào | 4.2.1 mask bits | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |
| TC-07 | Chỉ trace `close` (grader) | `trace 2097152 grep hello README` | `N: syscall close -> 0`, không có dòng `read` | `grade-lab-syscall` test_trace_close_grep | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |
| TC-08 | Trace `exec` + `open` (grader) | `trace 32896 grep hello README` | `N: syscall exec -> 3`, `N: syscall open -> 3`, không có `read` | `grade-lab-syscall` test_trace_exec_open_grep | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |
| TC-09 | `exec` thất bại được trace với giá trị âm | `trace 128 nosuchcmd` | `N: syscall exec -> -1` rồi `trace: exec failed` (`user/trace.c`) | 4.2.1 "return value" | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |
| TC-10 | Sai cú pháp | `trace grep` | `Usage: trace mask command` (`user/trace.c`) | — | docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log | PASS |

**Kết quả chạy thực tế (Ubuntu VM, 2026-11-03):** make clean && make qemu build và boot tới shell thành công. TC-01..TC-10 đều PASS trong một lần chạy, transcript tại docs/test-evidence/tracing/manual-qemu-final-2026-11-03.log. TC-04 dùng đúng mask 2 để chọn syscall fork (số 1); TC-01/02 trên README hiện tại trả về các lần read: 1023, 991, 324, 0. Trong make grade, cả sáu test tracing của grader đều báo OK (log: docs/test-evidence/tracing/make-grade-2026-11-03.log); tổng điểm 30/50 vì các mục ngoài tracing answers-syscall.txt, time.txt không có và attack thiếu secret mong đợi. Kết quả chung của grader vì vậy FAIL, không phải tất cả project đã qua.

### 1.4 procinfo (`procinfo` syscall)

Brief không cung cấp chương trình test. Bảng dưới giả định một chương trình user tên `procinfotest`.
Tên và giao diện của chương trình này là **DECISION NEEDED**.
`state` lấy theo `enum procstate` trong `kernel/proc.h`: UNUSED=0, USED=1, SLEEPING=2, RUNNABLE=3,
RUNNING=4, ZOMBIE=5.

| ID | Purpose | Command(s) typed in the xv6 shell | Expected output | Requirement in brief | Evidence file | Result |
|----|---------|-----------------------------------|-----------------|----------------------|---------------|--------|
| PI-01 | Ví dụ của brief: PID của chính nó, `procinfo(getpid(), &info)` | `procinfotest` | Trả về `0`. `Process: procinfotest`, `PID: 3, PPID: 2` (ngay sau boot, cha là `sh`), state = 4 (RUNNING). Dòng `State:`: brief dùng `printf("%s", info.state)` với kiểu `int`, đây là lỗi và sẽ crash hoặc in rác, nên cách in state là **DECISION NEEDED**. `Memory: N bytes` với `N == (uint64)sbrk(0)` (`sz` của process). Brief dùng `%d` cho `uint64`; printf của xv6 có hỗ trợ `%ld`. | 4.2.2 Usage Example | `docs/test-evidence/procinfo/PI-01.png` | |
| PI-02 | PID của cha (lấy từ `info.ppid` của PI-01, vì xv6 không có `getppid`) | `procinfotest` | Trả về `0`. name `sh`, pid 2, ppid 1, state 2 (SLEEPING, vì `sh` đang `wait(0)`) | 4.2.2 "parent process ID" | `docs/test-evidence/procinfo/PI-02.png` | |
| PI-03 | `init` (pid 1) không có cha | `procinfotest` | Trả về `0`, name `init`, state 2. Giá trị `ppid` khi không có cha: **DECISION NEEDED** (đề xuất `0`) | 4.2.2 | `docs/test-evidence/procinfo/PI-03.png` | |
| PI-04 | PID không hợp lệ: `9999`, `0`, `-1` | `procinfotest` | Trả về khác `0` (brief dùng `== 0` cho thành công; đề xuất `-1`). `info` không bị ghi. Kernel không panic. | 4.2.2 Signature | `docs/test-evidence/procinfo/PI-04.png` | |
| PI-05 | Con trỏ không hợp lệ: `(struct procinfo*)0` (trang text chỉ đọc) và `0xffffffffffffffff` (≥ MAXVA) | `procinfotest` | Trả về `-1` (`copyout` trong `kernel/vm.c` trên `syscall` từ chối trang không có `PTE_W` và địa chỉ ≥ MAXVA). Process **không** bị kill, kernel không panic. Kernel phải dùng `copyout`, không ghi trực tiếp. | 4.2.2 arg 2 "pointer" | `docs/test-evidence/procinfo/PI-05.png` | |
| PI-06 | Process ZOMBIE: `fork`, con `exit`, cha gọi `procinfo(childpid)` trước khi `wait` | `procinfotest` | Trả về `0`, state 5 (ZOMBIE), ppid = pid của cha | 4.2.2 "state" | `docs/test-evidence/procinfo/PI-06.png` | |

---

## 2. Quy tắc Git

### 2.1 Branch

- Mỗi bài một branch: `feat/xargs`, `feat/tree`, `feat/tracing`, `feat/procinfo`.
  `feat/xargs` đã tồn tại và hiện trùng với `util`.
- Branch tài liệu: `docs/<chủ-đề>`, ví dụ `docs/test-plan`.
- Branch gốc của từng `feat/*` phụ thuộc phương án tích hợp ở mục 3 (**DECISION NEEDED (team)**).
- Không code trực tiếp trên `main` (theo `AGENTS.md`).

### 2.2 Commit message

Dạng `<type>(<scope>): <mô tả ngắn, mệnh lệnh>`, viết tiếng Anh, không quá ~72 ký tự, không có dấu
chấm cuối.

- `type`: `feat`, `fix`, `test`, `docs`, `refactor`, `chore`.
- `scope`: `xargs`, `tree`, `tracing`, `procinfo`.
- Ví dụ: `feat(xargs): run command once per input line`,
  `fix(tree): skip entries with inum 0`, `docs: add test plan and git workflow`.
- Mỗi commit build được (`make qemu`).

### 2.3 Pull Request

- Một PR chỉ chứa **một bài**.
- Reviewer bắt buộc: xargs do Diễm Thúy review, tree do Trung Kiên review, tracing do Đăng Khoa review, procinfo do Khôi Nguyên review.
- Cần ít nhất 1 approve từ reviewer bắt buộc trước khi merge. Tác giả không tự merge khi chưa có
  approve.
- Không `git push --force` hoặc `--mirror` lên branch dùng chung (`main`, `util`, `syscall`).
  Branch `feat/*` cá nhân chỉ được force-push trước khi có người review.
- Thay đổi file syscall dùng chung (`kernel/syscall.h`, `kernel/syscall.c`, `user/user.h`,
  `user/usys.pl`, `Makefile`) phải báo trước cho nhóm (`AGENTS.md`). tracing và procinfo cùng sửa
  các file này.

Template mô tả PR:

```markdown
## What changed
- <file>: <thay đổi chính>

## How tested
- make clean && make qemu
- <các lệnh đã chạy>

## Test IDs passed
- XA-01 PASS, XA-02 PASS, ...
- (liệt kê cả FAIL / N/A kèm lý do)

## Screenshots
- docs/test-evidence/<exercise>/<TEST-ID>.png

## Open questions / limitations
- <DECISION NEEDED liên quan, giới hạn đã biết>
```

### 2.4 File không được commit

| File | `.gitignore` đã có? |
|------|---------------------|
| `*.o`, `*.d`, `*.asm`, `*.sym` | Có (`*.o`, `*.d`, `*.asm`, `*.sym`) |
| `user/_*` | Có (pattern `_*`, áp dụng cho mọi file có tên bắt đầu bằng `_`) |
| `fs.img` | Có (`*.img`) |
| `kernel/kernel` | Có |
| `user/usys.S` (sinh tự động) | Có |
| `xv6.out*`, `*.zip` | Có |
| `.env` | **Chưa** |
| Ghi chú cá nhân (ví dụ `notes.md`, `TODO.txt`) | **Chưa** |
| `*.patch` (file nộp bài) | **Chưa** |
| `.vscode/` | **Chưa** |

Các mục **chưa** có cần được thêm vào `.gitignore` bằng một PR riêng; tài liệu này không sửa
`.gitignore`. Trước khi commit, chạy `git status` và `git diff --cached --stat`, chỉ `git add` từng
file theo tên.

### 2.5 Bằng chứng test

- Screenshot: `docs/test-evidence/<exercise>/<TEST-ID>.png`, trong đó `<exercise>` là một trong
  `xargs`, `tree`, `tracing`, `procinfo`. Ví dụ: `docs/test-evidence/tree/TR-05.png`.
- Log tuỳ chọn: `docs/test-evidence/<exercise>/qemu-log.txt`, chép từ terminal QEMU.
  `xv6.out*` đã bị ignore nên không dùng tên này.
- Ảnh phải thấy được lệnh đã gõ và toàn bộ output tới dấu `$` tiếp theo.

---

## 3. Tích hợp và tạo patch nộp bài

### 3.1 Vấn đề

`util` và `syscall` tách nhau từ commit `dab0032`. `util` có 6 commit riêng (Util lab, xargstest,
stimecmp/menvcfg, `USERSTACK=2`). `syscall` có 5 commit riêng (syscall lab, `PROJECT.md`, …), và
`main` = `syscall` + `AGENTS.md`/`.github`.
`git diff util syscall --stat` cho thấy 16 file khác nhau: `README`, `conf/lab.mk`, `kernel/param.h`,
`kernel/riscv.h`, `kernel/vm.c`, `kernel/kalloc.c`, `grade-lab-util`/`grade-lab-syscall`,
`user/xargstest.sh`, `user/trace.c`, `user/sysinfotest.c`, `user/attack*.c`, `user/secret.c`, ….
Brief yêu cầu đúng **một** file `git diff`, mà một diff chỉ có nghĩa khi so với một base duy nhất.
Vì vậy code xargs/tree (làm trên `util`) và code tracing/procinfo (làm trên `syscall`) phải được đưa
về chung một base, nếu không patch sẽ kéo theo cả khác biệt giữa hai lab hoặc không apply được.
Điểm thuận lợi: `Makefile` giống hệt nhau trên hai branch, và xargs/tree chỉ thêm file mới trong
`user/` cùng dòng `UPROGS`, nên nhiều khả năng apply được lên `syscall`. Cần xác nhận bằng
`git apply --check`.

### 3.2 Phương án — **DECISION NEEDED (team)**

**Phương án A: một base duy nhất `syscall` (qua `main`)**

- Cách làm: cả 4 branch `feat/*` tách từ `main`. Riêng `feat/xargs` hiện đang ở `util`, cần tạo lại
  từ `main` hoặc cherry-pick commit sang. Merge từng PR vào `main`.
- Tạo patch:
  `git diff syscall main -- . ':(exclude)docs' ':(exclude)AGENTS.md' ':(exclude).github' > StudentID1_StudentID2_StudentID3.patch`
  Kiểm tra: `git checkout syscall` trong một clone sạch, `git apply --check <patch>`, `make qemu`,
  chạy lại toàn bộ test.
- Source zip: `make clean` trên `main` rồi nén.
- Ưu điểm: một base, một patch apply sạch lên đúng branch mà brief yêu cầu checkout (`syscall`).
  Source zip và patch khớp nhau.
- Nhược điểm: xargs/tree không được test trên `util` như brief mô tả (`USERSTACK=1` thay vì 2). Không
  có `grade-lab-util` và `xargstest.sh` trên base này, nên XA-14 chỉ chạy được bằng cách checkout
  `util` riêng.

**Phương án B: phát triển trên hai base, gộp thành một file patch**

- Cách làm: xargs/tree phát triển và test trên `util` (`feat/xargs`, `feat/tree`), tracing/procinfo
  trên `syscall`.
- Tạo patch:
  `{ git diff util <util-final> ; git diff syscall <syscall-final> -- . ':(exclude)docs' ':(exclude)AGENTS.md' ':(exclude).github' ; } > StudentID1_StudentID2_StudentID3.patch`
  Trong đó `<util-final>` là nhánh `util` đã merge xargs và tree, `<syscall-final>` là nhánh
  `syscall` đã merge tracing và procinfo.
- Source zip: phải chọn một trong hai cây (hoặc nộp cả hai). Brief chỉ nói "Zip file of xv6", nên
  cách này chưa rõ.
- Ưu điểm: mỗi bài được test đúng trên branch mà brief chỉ định, kèm grader tương ứng.
- Nhược điểm: file patch không tương ứng với một lệnh `git diff` duy nhất như ví dụ trong brief. Cả
  hai phần đều sửa `UPROGS` trong `Makefile`, nên khi apply toàn bộ lên một cây sẽ có hunk trùng hoặc
  xung đột. Source zip không khớp hoàn toàn với patch.

---

## 4. Definition of done

Áp dụng cho mọi bài:
- [ ] `make clean && make qemu` build không có warning mới.
- [ ] Mọi test ID của bài có Result khác rỗng, ảnh bằng chứng đã commit vào `docs/test-evidence/`.
- [ ] PR được reviewer bắt buộc approve, merge theo phương án ở mục 3.
- [ ] Không có file build hoặc file cá nhân trong commit (mục 2.4).
- [ ] Tác giả giải thích được từng dòng code (phỏng vấn).

**xargs**
- [ ] `user/xargs.c` và `$U/_xargs` trong `UPROGS`.
- [ ] XA-01 → XA-13 đạt. Các DECISION NEEDED của xargs đã được chốt và ghi vào báo cáo.
- [ ] Dùng `fork`/`exec`/`wait`, đọc từng ký tự tới `\n`, không ghi tràn `argv[MAXARG]`.

**tree**
- [ ] `user/tree.c` và `$U/_tree` trong `UPROGS`.
- [ ] TR-01 và TR-05 khớp từng ký tự với bảng (thụt lề 2 khoảng trắng mỗi cấp, `/` sau tên thư mục).
- [ ] TR-02 → TR-10 đạt. Không đệ quy vào `.` và `..`, kiểm tra độ dài path trước khi nối.

**tracing**
- [ ] `trace` có trong `kernel/syscall.h`, `kernel/syscall.c` (cả mảng tên syscall), `kernel/sysproc.c`,
  `kernel/proc.h`, `user/user.h`, `user/usys.pl`, và `$U/_trace` trong `UPROGS`.
- [ ] `fork()` copy mask sang con. Process mới không thừa hưởng mask cũ khi slot proc được dùng lại.
- [ ] TC-01 → TC-10 đạt. Các test trace trong `make grade` (`grade-lab-syscall`) đạt.

**procinfo**
- [ ] `struct procinfo` được khai báo một lần và dùng chung cho user và kernel. Syscall được đăng ký ở
  đủ các file như tracing, và có tên trong mảng tên syscall của tracing.
- [ ] Dùng `copyout`. Duyệt bảng `proc` có giữ lock đúng như `kernel/proc.c`. Đọc `p->parent` khi
  giữ `wait_lock`.
- [ ] PI-01 → PI-06 đạt.

---

## 5. Open decisions

| Mục | Đề xuất | Lý do | Ảnh hưởng tới |
|-----|---------|-------|---------------|
| D-01 `-n 1` | Chấp nhận và bỏ qua giá trị | Để ví dụ thứ hai của đề chạy được | XA-02 |
| D-02 dòng trống | Bỏ qua | Không có từ nào để nối thêm | XA-07 |
| D-03 dòng cuối không `\n` | Vẫn chạy | Dữ liệu hợp lệ, UNIX cũng xử lý như vậy | XA-08 |
| D-04 `exec` lỗi | In `xargs: exec <cmd> failed`, chạy tiếp dòng sau | Một dòng lỗi không làm hỏng các dòng khác | XA-09 |
| D-05 thiếu lệnh | In usage, `exit(1)` | Rõ ràng hơn mặc định `echo` | XA-10 |
| D-06 quá 31 đối số | In lỗi, `exit(1)` | Đúng với code trong tài liệu ôn tập; không cắt bớt âm thầm | XA-13 |
| D-07 dòng gốc của tree | In đường dẫn như người dùng gõ, thêm `/` (`a/`, `./`); riêng `/` in `/` | Khớp ví dụ `a/` của đề | TR-02, TR-03, TR-04, TR-10 |
| D-08 lỗi mở thư mục | `tree: cannot open <path>` | Giống phong cách thông báo lỗi của `ls.c` | TR-06 |
| D-09 tree trên file | In tên file rồi thoát | Hành vi giống `tree` trên Linux | TR-07 |
| D-10 chương trình test procinfo | `user/procinfotest.c` | Tên tạm của Claude là hợp lý | PI-01 → PI-06 |
| D-11 in state | In `%d`, kèm chú thích tên trạng thái theo `enum procstate` | Sửa lỗi trong ví dụ của đề; ghi lý do vào report | PI-01 |
| D-12 `ppid` của `init` | Trả về `0` | `init` không có process cha | PI-03 |
| D-13 số syscall | Chốt trước: `SYS_trace` và `SYS_procinfo` là hai số kế tiếp sau syscall cuối cùng trong `kernel/syscall.h` của nhánh `syscall` (hiện là 22 và 23, sau `SYS_close` = 21) | Tránh conflict khi merge hai PR | tracing, procinfo |
| D-14 tích hợp | Phương án A: mọi thứ dựa trên nhánh `syscall` | Ra đúng một `git diff` sạch như đề yêu cầu; xargs và tree chỉ thêm file trong `user/` và vài dòng `Makefile`, nên chuyển sang dễ dàng | mục 2.1, 3, patch nộp bài |
