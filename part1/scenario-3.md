I will implement RAID 10 the combination of RAID 1 (Mirroring) and RAID 0 (Stripping).
RAID 10 is suitable for this scenario because of the performance, reliability, and data integrity concerns. To solve the performance problem. I suggest stripping the mirror disk by using RAID 10 since mirroring increases the reliability if one disk is down we have a spare disk and stripping increases the performance for parallel writing.

For the hardware requirement, I suggest using 10 1-TB NAND Flash SSDs for primary storage for active transactions because is it drastically faster according to the SYS-205:00020 assessment. And also use 90 1-TB HDDs for secondary storage for older transactions. Implementing this requires more budget. On the other hand, the performance will become faster.

To ensure data integrity, Since RAID 10 was implemented, we can store logs in NVRAM before committing to disk to protect data against power failures, controller failures, and partial writes. 