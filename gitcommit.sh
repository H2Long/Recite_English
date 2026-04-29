#!/bin/bash
# Git 提交脚本
# 用法: ./gitcommit.sh "提交信息"

if [ -z "$1" ]; then
    echo "错误: 请提供提交信息"
    echo "用法: $0 \"提交信息\""
    exit 1
fi

git add -A
git commit -m "$1"
git push
