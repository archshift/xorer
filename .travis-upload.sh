if [ "$TRAVIS_BRANCH" = "master" ]; then
    GITDATE="`git show -s --date=short --format='%ad' | sed 's/-//g'`"
    GITREV="`git show -s --format='%h'`"

    if [ "$TRAVIS_OS_NAME" = "linux" -o -z "$TRAVIS_OS_NAME" ]; then
        REV_NAME="citra-${GITDATE}-${GITREV}-linux-amd64"
        UPLOAD_DIR="/xorer/nightly/linux-amd64"
        
        sudo apt-get -qq install lftp
    elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
        REV_NAME="citra-${GITDATE}-${GITREV}-osx-amd64"
        UPLOAD_DIR="/xorer/nightly/osx-amd64"
        
        brew install lftp
    fi
    
    mkdir "$REV_NAME"
    cp target/release/xorer "$REV_NAME"
    ARCHIVE_NAME="${REV_NAME}.tar.xz"
    tar -cJvf "$ARCHIVE_NAME" "$REV_NAME"
    lftp -c "open -u builds,$BUILD_PASSWORD sftp://builds.archshift.com; put -O '$UPLOAD_DIR' '$ARCHIVE_NAME'"
fi
