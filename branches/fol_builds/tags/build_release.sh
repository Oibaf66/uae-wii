#!/bin/sh

if [ $# -lt 1 ]; then
    echo "Usage: build_release.sh DIR"

    exit 1
fi

DIR=`basename $1`

# Exclude generated stuff and svn things
tar --exclude="*~" --exclude="*.pyc" --exclude="*.pyo" --exclude=".svn*" -czf $DIR.tar.gz $DIR
