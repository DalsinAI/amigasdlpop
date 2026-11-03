# Build receipts

Each file is a `receipt.json` (or `data-receipt.json`) exactly as the build
wrote it, byte for byte (its SHA-256 is in the table), kept here because the
build output itself is not committed. `repository_commit` is the commit that
was built; the receipt is committed after it.

| File | Build | Where | Receipt SHA-256 |
|---|---|---|---|
| `1-build-3e9b058-open5-stove_build.json` | Cradle `stove_build` of `3e9b058`, target `.` (job `j20261103T013956-4a8693`; 0001 only) | Cradle's `os32-gcc16` stove (open5) | `208982e5f5e3506e4757be6d3e1eaa8c3e4b0c8b123647f037344004087055d9` |
| `1-build-open5-stove_build.json` | Cradle `stove_build` of `65372f8`, target `.` (job `j20261103T015531-934dcf`) | Cradle's `os32-gcc16` stove (open5) | `83c6a51e92c3a420866c217f2e2d20a98b645fc17327996476825fe6bfd8a0e2` |
| `1-data.json` | Cradle `stove_build` of `1036067`, target `runtime-data` (job `j20261103T014106-460abd`; lab use only) | Cradle (open5) | `e39c3ab7c4e25d759e02b4dd2df6a007f1bc4da626a246948bce6115e1001799` |
| `2-presentation-open5-stove_build.json` | Cradle `stove_build` of `6eb1593`, target `.` (job `j20261103T020043-d57fff`) | Cradle's `os32-gcc16` stove (open5) | `ed849cdeb05f3654879e75dc254294d10755c32c480761469008eaa5eba72521` |
