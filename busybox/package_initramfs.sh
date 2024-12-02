if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <src_dir> <build_dir> <busybox_install_dir> <output>"
    exit 1
fi

src_dir=$1
build_dir=$2
busybox_install_dir=$3
output=$4

echo -e "Creating initramfs with arguments:\n \tsrc_dir=\033[1;32m${src_dir}\033[0m,\n \tbuild_dir=\033[1;32m${build_dir}\033[0m,\n \tbusybox_install_dir=\033[1;32m${busybox_install_dir}\033[0m,\n \toutput=\033[1;32m${output}\033[0m"

rm -f ${output}
echo "Creating Minimal Filesystem"
mkdir -p ${build_dir}/initramfs/busybox-x86
cd ${build_dir}/initramfs/busybox-x86
mkdir -pv bin sbin etc proc sys usr
echo "Copying BusyBox file"
cp -av ${busybox_install_dir}/* .
echo "Setting up init"

cp ${src_dir}/init.ref init
chmod +x init

echo "ENV=/.shinit; export ENV" > .profile

cp ${src_dir}/shinit.ref .shinit
chmod +x .shinit


find . -print0 | cpio --null -ov --format=newc | gzip -9 > ${output}