# Runtime IR statistics

Static instructions: 179; phi Users skipped: 4; executed sites: 175.

Instruction events: 20785350; User <- Operand events: 35199042.

Windows overlap and continue across blocks, functions and frames. Instruction operands are listed in operand-index order. Constants, arguments, globals and basic blocks are excluded from use events.

## Instructions

### Window 1

Total: 20785350; distinct: 14.

| Count | Share | Pattern |
|---:|---:|:---|
| 4202566 | 20.2189% | `call` |
| 2893824 | 13.9224% | `icmp` |
| 2369536 | 11.4000% | `add` |
| 2101276 | 10.1094% | `getelementptr` |
| 1839104 | 8.8481% | `load` |
| 1581056 | 7.6066% | `or` |
| 1572864 | 7.5672% | `sext` |
| 1572864 | 7.5672% | `select` |
| 1048608 | 5.0449% | `store` |
| 528384 | 2.5421% | `shl` |

### Window 2

Total: 20785349; distinct: 47.

| Count | Share | Pattern |
|---:|---:|:---|
| 2361349 | 11.3606% | `call; call` |
| 1839104 | 8.8481% | `getelementptr; load` |
| 1579008 | 7.5967% | `or; call` |
| 1576960 | 7.5869% | `call; or` |
| 1310720 | 6.3060% | `icmp; select` |
| 1058816 | 5.0940% | `add; icmp` |
| 1048576 | 5.0448% | `sext; getelementptr` |
| 786432 | 3.7836% | `load; add` |
| 786432 | 3.7836% | `select; icmp` |
| 524288 | 2.5224% | `add; add` |

### Window 3

Total: 20785348; distinct: 73.

| Count | Share | Pattern |
|---:|---:|:---|
| 1576960 | 7.5869% | `call; or; call` |
| 1572868 | 7.5672% | `call; call; call` |
| 790528 | 3.8033% | `or; call; or` |
| 788480 | 3.7934% | `or; call; call` |
| 786432 | 3.7836% | `getelementptr; load; add` |
| 786432 | 3.7836% | `sext; getelementptr; load` |
| 786432 | 3.7836% | `icmp; select; icmp` |
| 524288 | 2.5224% | `add; add; icmp` |
| 524288 | 2.5224% | `add; icmp; select` |
| 524288 | 2.5224% | `load; sext; getelementptr` |

### Window 4

Total: 20785347; distinct: 94.

| Count | Share | Pattern |
|---:|---:|:---|
| 790528 | 3.8033% | `or; call; or; call` |
| 788480 | 3.7934% | `call; or; call; or` |
| 788480 | 3.7934% | `call; or; call; call` |
| 786435 | 3.7836% | `call; call; call; call` |
| 786432 | 3.7836% | `or; call; call; call` |
| 524288 | 2.5224% | `add; icmp; select; icmp` |
| 524288 | 2.5224% | `getelementptr; load; sext; getelementptr` |
| 524288 | 2.5224% | `zext; icmp; sext; add` |
| 524288 | 2.5224% | `sext; add; add; icmp` |
| 524288 | 2.5224% | `sext; getelementptr; load; add` |

### Window 5

Total: 20785346; distinct: 111.

| Count | Share | Pattern |
|---:|---:|:---|
| 788480 | 3.7934% | `call; or; call; or; call` |
| 786432 | 3.7836% | `or; call; call; call; call` |
| 786432 | 3.7836% | `call; or; call; call; call` |
| 526336 | 2.5322% | `or; call; or; call; or` |
| 524288 | 2.5224% | `zext; icmp; sext; add; add` |
| 524288 | 2.5224% | `icmp; zext; icmp; sext; add` |
| 524288 | 2.5224% | `icmp; sext; add; add; icmp` |
| 524288 | 2.5224% | `icmp; select; icmp; select; store` |
| 524288 | 2.5224% | `call; call; or; call; call` |
| 524288 | 2.5224% | `call; call; call; or; call` |

## User <- Operand

### Window 1

Total: 35199042; distinct: 32.

| Count | Share | Pattern |
|---:|---:|:---|
| 6307840 | 17.9205% | `call <- or` |
| 4194304 | 11.9160% | `call <- load` |
| 2097152 | 5.9580% | `call <- shl` |
| 1839104 | 5.2249% | `load <- getelementptr` |
| 1835008 | 5.2132% | `add <- load` |
| 1581056 | 4.4918% | `or <- shl` |
| 1572864 | 4.4685% | `select <- icmp` |
| 1320960 | 3.7528% | `icmp <- add` |
| 1314844 | 3.7355% | `getelementptr <- alloca` |
| 1310720 | 3.7237% | `select <- add` |

### Window 2

Total: 35199041; distinct: 72.

| Count | Share | Pattern |
|---:|---:|:---|
| 3145728 | 8.9370% | `call <- or; call <- load` |
| 2369536 | 6.7318% | `call <- or; call <- or` |
| 2359296 | 6.7027% | `call <- load; call <- or` |
| 1572864 | 4.4685% | `call <- load; or <- shl` |
| 1052672 | 2.9906% | `getelementptr <- alloca; getelementptr <- phi` |
| 1052672 | 2.9906% | `getelementptr <- phi; load <- getelementptr` |
| 1048576 | 2.9790% | `call <- shl; call <- load` |
| 1048576 | 2.9790% | `select <- icmp; select <- add` |
| 792576 | 2.2517% | `or <- shl; call <- or` |
| 786432 | 2.2342% | `or <- shl; call <- shl` |

### Window 3

Total: 35199040; distinct: 97.

| Count | Share | Pattern |
|---:|---:|:---|
| 2359296 | 6.7027% | `call <- or; call <- or; call <- load` |
| 2359296 | 6.7027% | `call <- or; call <- load; call <- or` |
| 2359296 | 6.7027% | `call <- load; call <- or; call <- or` |
| 1052672 | 2.9906% | `getelementptr <- alloca; getelementptr <- phi; load <- getelementptr` |
| 1048576 | 2.9790% | `call <- shl; call <- load; or <- shl` |
| 786432 | 2.2342% | `or <- shl; call <- shl; call <- or` |
| 786432 | 2.2342% | `or <- shl; call <- or; call <- shl` |
| 786432 | 2.2342% | `icmp <- add; select <- icmp; select <- add` |
| 786432 | 2.2342% | `call <- shl; call <- or; call <- load` |
| 786432 | 2.2342% | `call <- or; call <- shl; call <- load` |

### Window 4

Total: 35199039; distinct: 117.

| Count | Share | Pattern |
|---:|---:|:---|
| 2359296 | 6.7027% | `call <- or; call <- load; call <- or; call <- or` |
| 2359296 | 6.7027% | `call <- load; call <- or; call <- or; call <- load` |
| 1572864 | 4.4685% | `call <- or; call <- or; call <- load; call <- or` |
| 786432 | 2.2342% | `or <- shl; call <- shl; call <- or; call <- load` |
| 786432 | 2.2342% | `or <- shl; call <- or; call <- shl; call <- load` |
| 786432 | 2.2342% | `call <- shl; call <- or; call <- load; call <- or` |
| 786432 | 2.2342% | `call <- shl; call <- load; or <- shl; call <- or` |
| 786432 | 2.2342% | `call <- or; call <- shl; call <- load; or <- shl` |
| 786432 | 2.2342% | `call <- load; or <- shl; call <- shl; call <- or` |
| 786432 | 2.2342% | `call <- load; or <- shl; call <- or; call <- shl` |

### Window 5

Total: 35199038; distinct: 128.

| Count | Share | Pattern |
|---:|---:|:---|
| 2359296 | 6.7027% | `call <- or; call <- load; call <- or; call <- or; call <- load` |
| 1572864 | 4.4685% | `call <- or; call <- or; call <- load; call <- or; call <- or` |
| 1572864 | 4.4685% | `call <- load; call <- or; call <- or; call <- load; call <- or` |
| 786432 | 2.2342% | `or <- shl; call <- shl; call <- or; call <- load; call <- or` |
| 786432 | 2.2342% | `or <- shl; call <- or; call <- shl; call <- load; or <- shl` |
| 786432 | 2.2342% | `call <- shl; call <- or; call <- load; call <- or; call <- or` |
| 786432 | 2.2342% | `call <- shl; call <- load; or <- shl; call <- or; call <- shl` |
| 786432 | 2.2342% | `call <- load; or <- shl; call <- shl; call <- or; call <- load` |
| 786432 | 2.2342% | `call <- load; or <- shl; call <- or; call <- shl; call <- load` |
| 524288 | 1.4895% | `zext <- icmp; icmp <- and; sext <- icmp; add <- load; add <- zext` |
