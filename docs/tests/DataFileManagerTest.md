[Directory](/tests/tools/DataFileManagerTest.cpp)

# DataFileManagerTest.cpp

## **1** Classes

- DataFileManager  
  - Fully tested class responsible for CSV-style file output with dynamic columns.

---

## **2** Test Overview

- File creation and opening (`Open`)
- Safe closing (`Close`)
- Column registration (`AddColumn`)
- CSV header generation (written once)
- Row writing via `Update`
- Column execution order correctness
- Lambda state capture correctness
- File truncation on reopen
- Safe repeated `Close()` calls

Type:
- Unit tests with filesystem interaction

---

## **3** Failure Conditions

- File cannot be created or opened
- Header missing or duplicated
- Incorrect CSV formatting
- Column order mismatch
- Incorrect row output
- Reopen fails to truncate file
- Unsafe `Close()` behavior
- Lambda capture/state issues

---

## **4** Bug Log

- None reported