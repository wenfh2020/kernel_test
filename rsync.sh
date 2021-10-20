#!/bin/sh
# rsync code from mac to linux.
work_path=$(dirname $0)
cd $work_path

src=.
dst=root@lu14:/root/linux-5.0.1/kernel_test/
echo "$src --> $dst"

rsync -ravz --exclude=".git/" \
            --exclude=".vscode/" \
            --include="*.c" \
            --include="Makefile" \
            --include="*.md" \
            --include="*.h" \
            --include="*/" \
            --exclude="*" \
            $src $dst
