const { test } = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const { spawnSync } = require('node:child_process');

const root = path.resolve(__dirname, '../..');
const boards = ['cv1842hp_ovis_spinand', 'cv1842hp_ovis_cvbs_spinand'];
const overlay = (board, name) => path.join(root, 'ramdisk/rootfs/overlay', board, 'etc/init.d', name);

function executable(file, content) {
  fs.writeFileSync(file, content, { mode: 0o755 });
}

for (const board of boards) {
  test(`${board}: startup scripts parse`, () => {
    for (const name of ['S77ncm', 'S99v_ovis_usb', 'S99z_ipcamera']) {
      const result = spawnSync('sh', ['-n', overlay(board, name)], { encoding: 'utf8' });
      assert.equal(result.status, 0, result.stderr);
    }
  });

  for (const scenario of ['ready', 'retry', 'failed', 'missing-mpp']) {
    test(`${board}: USB ${scenario}`, () => {
      const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'ovis-startup-test-'));
      try {
        fs.mkdirSync(path.join(dir, 'run'));
        fs.mkdirSync(path.join(dir, 'gadget'));
        if (scenario !== 'missing-mpp') fs.writeFileSync(path.join(dir, 'run/ovis-mpp-ready'), '');
        executable(path.join(dir, 'manager'), '#!/bin/sh\nshift 3\nexec "$@"\n');
        executable(path.join(dir, 'usb'), `#!/bin/sh
count=$(cat '${dir}/count' 2>/dev/null || echo 0)
count=$((count + 1))
echo "$count" > '${dir}/count'
${scenario === 'failed' ? 'exit 1' : ''}
${scenario === 'retry' ? '[ "$count" -ne 1 ] || exit 1' : ''}
echo udc0 > '${dir}/gadget/UDC'
`);
        const source = fs.readFileSync(overlay(board, 'S99v_ovis_usb'), 'utf8')
          .replace('/usr/sbin/ovis-managerd', `${dir}/manager`)
          .replaceAll('/etc/init.d/S77ncm', `${dir}/usb`)
          .replaceAll('/var/log', `${dir}/log`)
          .replaceAll('/var/run', `${dir}/run`)
          .replaceAll('/tmp/usb/usb_gadget/cvitek', `${dir}/gadget`)
          .replace(/\/dev\/soph-(vi|vpss|rgn)/g, '/dev/null');
        const result = spawnSync('sh', ['-c', `sleep() { :; }\n${source}`, 'test', 'start'],
          { encoding: 'utf8', timeout: 3000 });
        assert.equal(result.error, undefined);
        assert.equal(result.status, ['failed', 'missing-mpp'].includes(scenario) ? 1 : 0,
          result.stdout + result.stderr);
        const calls = fs.existsSync(`${dir}/count`) ? Number(fs.readFileSync(`${dir}/count`, 'utf8')) : 0;
        assert.equal(calls, { ready: 1, retry: 2, failed: 3, 'missing-mpp': 0 }[scenario]);
      } finally {
        fs.rmSync(dir, { recursive: true, force: true });
      }
    });
  }

  test(`${board}: network starts manager and camera concurrently`, () => {
    const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'ovis-services-test-'));
    try {
      executable(`${dir}/manager`, `#!/bin/sh
touch '${dir}/manager-started'
count=0
while [ ! -e '${dir}/camera-started' ] && [ "$count" -lt 50 ]; do
  sleep 0.01
  count=$((count + 1))
done
[ -e '${dir}/camera-started' ]
`);
      executable(`${dir}/camera`, `#!/bin/sh\ntouch '${dir}/camera-started'\n`);
      const source = fs.readFileSync(overlay(board, 'S77ncm'), 'utf8');
      const activate = source.slice(source.indexOf('activate_network() {'),
        source.indexOf('\nsetup_provisioning_gadget()'))
        .replace('/etc/init.d/S98ovis-manager', `${dir}/manager`)
        .replace('/etc/init.d/S99z_ipcamera', `${dir}/camera`);
      const result = spawnSync('sh', ['-c', `
detect_ncm_interface() { ncm_interface=usb0; }
load_network_subnet() { ncm_address=192.168.42.1; }
ifconfig() { :; }
start_dhcp() { :; }
NCM_ADDRESS_FILE='${dir}/address'
${activate}
activate_network
`], { encoding: 'utf8', timeout: 3000 });
      assert.equal(result.status, 0, result.stdout + result.stderr);
    } finally {
      fs.rmSync(dir, { recursive: true, force: true });
    }
  });
}
