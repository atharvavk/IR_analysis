; ModuleID = 'hello.c'
source_filename = "hello.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %c = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 10, i32* %a, align 4
  store i32 20, i32* %b, align 4
  store i32 0, i32* %c, align 4
  br label %while.cond

while.cond:                                       ; preds = %if.end, %entry
  %0 = load i32, i32* %b, align 4
  %1 = load i32, i32* %a, align 4
  %cmp = icmp sgt i32 %0, %1
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %2 = load i32, i32* %a, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* %a, align 4
  %3 = load i32, i32* %c, align 4
  %4 = load i32, i32* %a, align 4
  %cmp1 = icmp slt i32 %3, %4
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %while.body
  %5 = load i32, i32* %c, align 4
  %inc2 = add nsw i32 %5, 1
  store i32 %inc2, i32* %c, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %while.body
  br label %while.cond

while.end:                                        ; preds = %while.cond
  store i32 96, i32* %a, align 4
  %6 = load i32, i32* %b, align 4
  %7 = load i32, i32* %a, align 4
  %cmp3 = icmp eq i32 %6, %7
  br i1 %cmp3, label %if.then4, label %if.end6

if.then4:                                         ; preds = %while.end
  %8 = load i32, i32* %c, align 4
  %inc5 = add nsw i32 %8, 1
  store i32 %inc5, i32* %c, align 4
  br label %if.end6

if.end6:                                          ; preds = %if.then4, %while.end
  br label %while.cond7

while.cond7:                                      ; preds = %if.end14, %if.end6
  %9 = load i32, i32* %b, align 4
  %10 = load i32, i32* %a, align 4
  %cmp8 = icmp sgt i32 %9, %10
  br i1 %cmp8, label %while.body9, label %while.end15

while.body9:                                      ; preds = %while.cond7
  %11 = load i32, i32* %a, align 4
  %inc10 = add nsw i32 %11, 1
  store i32 %inc10, i32* %a, align 4
  %12 = load i32, i32* %c, align 4
  %13 = load i32, i32* %a, align 4
  %cmp11 = icmp slt i32 %12, %13
  br i1 %cmp11, label %if.then12, label %if.end14

if.then12:                                        ; preds = %while.body9
  %14 = load i32, i32* %c, align 4
  %inc13 = add nsw i32 %14, 1
  store i32 %inc13, i32* %c, align 4
  br label %if.end14

if.end14:                                         ; preds = %if.then12, %while.body9
  br label %while.cond7

while.end15:                                      ; preds = %while.cond7
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 11.0.0-2"}
