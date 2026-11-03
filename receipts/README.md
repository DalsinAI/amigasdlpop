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
| `3-input-331f1ed-open5-stove_build.json` | Cradle `stove_build` of `331f1ed`, target `.` (job `j20261103T020857-9b633d`; the first input build) | Cradle's `os32-gcc16` stove (open5) | `13f696347cc404a720063895fc3b90b4035844c495bfd5c830a8c9432f54c64c` |
| `3-input-open5-stove_build.json` | Cradle `stove_build` of `7935e72`, target `.` (job `j20261103T021336-3a33f5`) | Cradle's `os32-gcc16` stove (open5) | `89847a272e8c2569e2f519556293f9b6381cf982fd13c51e47f1df5850692744` |
| `4-sound-open5-stove_build.json` | Cradle `stove_build` of `cf04fb9`, target `.` (job `j20261103T022000-7e4ad6`) | Cradle's `os32-gcc16` stove (open5) | `0fdcef77d8b09d5103c7be2cfe5f7b35ff53a22072c36349619d23b038bd57e3` |
| `5-saves-open5-stove_build.json` | Cradle `stove_build` of `8b52ef0`, target `.` (job `j20261103T022540-340c41`) | Cradle's `os32-gcc16` stove (open5) | `08074a892ef3fff6d96e5f51d04045bb8d8d6419c4e1044d9decc76ad7bd76da` |
| `6-package-open5-stove_build.json` | Cradle `stove_build` of `3a06a94`, target `package` (job `j20261103T023514-1c0c68`; the receipt of the build in the package) | Cradle's `os32-gcc16` stove (open5) | `51108a51ace7a978ce9639a00105ec5f9324eaa86da677dfe4557fbb04bb9ec4` |
