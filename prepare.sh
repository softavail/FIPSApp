#!/bin/bash

CURRENT_PATH=`pwd`
DEST_PREBUILT_DIR="${CURRENT_PATH}/prebuilt"
DEST_DIR_NAME="arm64-v8a"
DEST_DIR="${DEST_PREBUILT_DIR}/$DEST_DIR_NAME"
BINARY_ARCHIVE_NAME="android-arm64-v8a-bin.tar.gz"
DOWNLOAD_LINK_FILE_ID="1dbhvEcToRnRd_vPCntoJ_fbzV6eS2A7n"
APP_RES_RAW_FOLDER="${CURRENT_PATH}/app/src/main/res/raw"


gdrive_download()
{
    FILE_ID="$1"
    OUTPUT="$2"

    # Temporary cookies file
    COOKIE_FILE=$(mktemp)

    # Step 1: Get the confirmation token if required
    CONFIRM=$(curl -sc "$COOKIE_FILE" "https://drive.google.com/uc?export=download&id=${FILE_ID}" \
        | sed -rn 's/.*confirm=([0-9A-Za-z_]+).*/\1/p')

    # Step 2: Download the file using the confirmation token
    curl -Lb "$COOKIE_FILE" "https://drive.google.com/uc?export=download&confirm=${CONFIRM}&id=${FILE_ID}" \
        -o "$OUTPUT"

    echo "Downloaded: $OUTPUT"
}

download()
{
    if [ ! -e ${BINARY_ARCHIVE_NAME} ]; then
        echo "Downloading ${BINARY_ARCHIVE_NAME}"
        gdrive_download "${DOWNLOAD_LINK_FILE_ID}" "${BINARY_ARCHIVE_NAME}"
    else
        echo "Using ${BINARY_ARCHIVE_NAME}"
    fi
}

copy_bin_to_res_raw()
{
    cp -f "$DEST_DIR/fips.so" "${APP_RES_RAW_FOLDER}/fips_module_arm64_v8a.so"
}

prepare()
{
   if [ ! -d "$DEST_PREBUILT_DIR" ]; then
       echo "Cannot find $DEST_PREBUILT_DIR"
       mkdir -p ${DEST_PREBUILT_DIR}

   fi

   cd ${DEST_PREBUILT_DIR}

   if [ ! -d "$DEST_DIR" ]; then
       echo "Cannot find $DEST_DIR - downloading it ..."
       
       download
       mkdir -p "$DEST_DIR"
       tar -zxvf "${BINARY_ARCHIVE_NAME}"
   else
       echo "Already prepared ... - nothing to do anymore!"
   fi
   
}


prepare

   cd ${CURRENT_PATH}

   copy_bin_to_res_raw

cd ${CURRENT_PATH}
