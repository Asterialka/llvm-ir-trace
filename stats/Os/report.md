# Runtime IR statistics

Static instructions: 140; phi Users skipped: 10; executed sites: 130.

Instruction events: 41494874; User <- Operand events: 59840790.

Windows overlap and continue across blocks, functions and frames. Instruction operands are listed in operand-index order. Constants, arguments, globals and basic blocks are excluded from use events.

## Instructions

### Window 1

Total: 41494874; distinct: 16.

| Count | Share | Pattern |
|---:|---:|:---|
| 12855344 | 30.9806% | `add` |
| 8136712 | 19.6090% | `icmp` |
| 6842439 | 16.4898% | `br` |
| 4202566 | 10.1279% | `call` |
| 2101280 | 5.0640% | `getelementptr` |
| 1839104 | 4.4321% | `load` |
| 1572864 | 3.7905% | `sext` |
| 1572864 | 3.7905% | `select` |
| 1048608 | 2.5271% | `store` |
| 528384 | 1.2734% | `shl` |

### Window 2

Total: 41494873; distinct: 55.

| Count | Share | Pattern |
|---:|---:|:---|
| 6301704 | 15.1867% | `add; icmp` |
| 6299648 | 15.1818% | `br; add` |
| 5515272 | 13.2915% | `icmp; br` |
| 4196352 | 10.1129% | `call; add` |
| 4194304 | 10.1080% | `add; call` |
| 1839104 | 4.4321% | `getelementptr; load` |
| 1310720 | 3.1588% | `icmp; select` |
| 1048576 | 2.5270% | `add; br` |
| 1048576 | 2.5270% | `sext; getelementptr` |
| 786432 | 1.8953% | `load; add` |

### Window 3

Total: 41494872; distinct: 86.

| Count | Share | Pattern |
|---:|---:|:---|
| 5515272 | 13.2915% | `add; icmp; br` |
| 4988928 | 12.0230% | `icmp; br; add` |
| 4196352 | 10.1129% | `call; add; icmp` |
| 4194304 | 10.1080% | `br; add; call` |
| 4194304 | 10.1080% | `add; call; add` |
| 1056768 | 2.5467% | `br; add; icmp` |
| 1048576 | 2.5270% | `br; add; br` |
| 1048576 | 2.5270% | `add; br; add` |
| 786432 | 1.8953% | `getelementptr; load; add` |
| 786432 | 1.8953% | `sext; getelementptr; load` |

### Window 4

Total: 41494871; distinct: 111.

| Count | Share | Pattern |
|---:|---:|:---|
| 4988928 | 12.0230% | `add; icmp; br; add` |
| 4196352 | 10.1129% | `call; add; icmp; br` |
| 4194304 | 10.1080% | `br; add; call; add` |
| 4194304 | 10.1080% | `add; call; add; icmp` |
| 3145728 | 7.5810% | `icmp; br; add; call` |
| 1056768 | 2.5467% | `br; add; icmp; br` |
| 1056768 | 2.5467% | `icmp; br; add; icmp` |
| 1048576 | 2.5270% | `br; add; br; add` |
| 1048576 | 2.5270% | `add; br; add; call` |
| 786432 | 1.8953% | `icmp; br; add; br` |

### Window 5

Total: 41494870; distinct: 133.

| Count | Share | Pattern |
|---:|---:|:---|
| 4194304 | 10.1080% | `br; add; call; add; icmp` |
| 4194304 | 10.1080% | `add; call; add; icmp; br` |
| 4194304 | 10.1080% | `call; add; icmp; br; add` |
| 3145728 | 7.5810% | `add; icmp; br; add; call` |
| 3145728 | 7.5810% | `icmp; br; add; call; add` |
| 1056768 | 2.5467% | `add; icmp; br; add; icmp` |
| 1056768 | 2.5467% | `icmp; br; add; icmp; br` |
| 1048576 | 2.5270% | `br; add; br; add; call` |
| 1048576 | 2.5270% | `add; br; add; call; add` |
| 786432 | 1.8953% | `br; add; icmp; br; add` |

## User <- Operand

### Window 1

Total: 59840790; distinct: 39.

| Count | Share | Pattern |
|---:|---:|:---|
| 10758176 | 17.9780% | `add <- phi` |
| 8388608 | 14.0182% | `call <- add` |
| 6563848 | 10.9689% | `icmp <- add` |
| 5515272 | 9.2166% | `br <- icmp` |
| 5242880 | 8.7614% | `add <- shl` |
| 4194304 | 7.0091% | `call <- load` |
| 1839104 | 3.0733% | `load <- getelementptr` |
| 1835008 | 3.0665% | `add <- load` |
| 1572864 | 2.6284% | `select <- icmp` |
| 1314848 | 2.1972% | `getelementptr <- alloca` |

### Window 2

Total: 59840789; distinct: 80.

| Count | Share | Pattern |
|---:|---:|:---|
| 5515272 | 9.2166% | `add <- phi; icmp <- add` |
| 5515272 | 9.2166% | `icmp <- add; br <- icmp` |
| 5242880 | 8.7614% | `add <- phi; add <- shl` |
| 4988928 | 8.3370% | `br <- icmp; add <- phi` |
| 4194304 | 7.0091% | `add <- shl; call <- add` |
| 4194304 | 7.0091% | `call <- add; call <- add` |
| 4194304 | 7.0091% | `call <- add; call <- load` |
| 4194304 | 7.0091% | `call <- load; add <- phi` |
| 1052704 | 1.7592% | `getelementptr <- alloca; getelementptr <- phi` |
| 1052672 | 1.7591% | `getelementptr <- phi; load <- getelementptr` |

### Window 3

Total: 59840788; distinct: 109.

| Count | Share | Pattern |
|---:|---:|:---|
| 5515272 | 9.2166% | `add <- phi; icmp <- add; br <- icmp` |
| 4988928 | 8.3370% | `icmp <- add; br <- icmp; add <- phi` |
| 4194304 | 7.0091% | `add <- shl; call <- add; call <- add` |
| 4194304 | 7.0091% | `add <- phi; add <- shl; call <- add` |
| 4194304 | 7.0091% | `call <- add; call <- add; call <- load` |
| 4194304 | 7.0091% | `call <- add; call <- load; add <- phi` |
| 4194304 | 7.0091% | `call <- load; add <- phi; icmp <- add` |
| 3932160 | 6.5710% | `br <- icmp; add <- phi; add <- shl` |
| 1056768 | 1.7660% | `br <- icmp; add <- phi; icmp <- add` |
| 1052672 | 1.7591% | `getelementptr <- alloca; getelementptr <- phi; load <- getelementptr` |

### Window 4

Total: 59840787; distinct: 131.

| Count | Share | Pattern |
|---:|---:|:---|
| 4988928 | 8.3370% | `add <- phi; icmp <- add; br <- icmp; add <- phi` |
| 4194304 | 7.0091% | `add <- shl; call <- add; call <- add; call <- load` |
| 4194304 | 7.0091% | `add <- phi; add <- shl; call <- add; call <- add` |
| 4194304 | 7.0091% | `call <- add; call <- add; call <- load; add <- phi` |
| 4194304 | 7.0091% | `call <- add; call <- load; add <- phi; icmp <- add` |
| 4194304 | 7.0091% | `call <- load; add <- phi; icmp <- add; br <- icmp` |
| 3932160 | 6.5710% | `icmp <- add; br <- icmp; add <- phi; add <- shl` |
| 3145728 | 5.2568% | `br <- icmp; add <- phi; add <- shl; call <- add` |
| 1056768 | 1.7660% | `br <- icmp; add <- phi; icmp <- add; br <- icmp` |
| 1056768 | 1.7660% | `icmp <- add; br <- icmp; add <- phi; icmp <- add` |

### Window 5

Total: 59840786; distinct: 145.

| Count | Share | Pattern |
|---:|---:|:---|
| 4194304 | 7.0091% | `add <- shl; call <- add; call <- add; call <- load; add <- phi` |
| 4194304 | 7.0091% | `add <- phi; add <- shl; call <- add; call <- add; call <- load` |
| 4194304 | 7.0091% | `call <- add; call <- add; call <- load; add <- phi; icmp <- add` |
| 4194304 | 7.0091% | `call <- add; call <- load; add <- phi; icmp <- add; br <- icmp` |
| 4194304 | 7.0091% | `call <- load; add <- phi; icmp <- add; br <- icmp; add <- phi` |
| 3932160 | 6.5710% | `add <- phi; icmp <- add; br <- icmp; add <- phi; add <- shl` |
| 3145728 | 5.2568% | `br <- icmp; add <- phi; add <- shl; call <- add; call <- add` |
| 3145728 | 5.2568% | `icmp <- add; br <- icmp; add <- phi; add <- shl; call <- add` |
| 1056768 | 1.7660% | `add <- phi; icmp <- add; br <- icmp; add <- phi; icmp <- add` |
| 1056768 | 1.7660% | `icmp <- add; br <- icmp; add <- phi; icmp <- add; br <- icmp` |
