const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const runTests = async () => {
    const logPath = path.join(__dirname, 'e2e.log')
    const rootPath = path.join(__dirname, '..')
    if (path.basename(process.cwd()) !== 'stellar') {
        console.warn('Warning: wrong working directory. Your working directory should be the root directory of the project (the stellar folder)')
    }
    if (await fs.existsSync(logPath)) {
        await fs.rmSync(logPath)
    }
    console.log('Compiling stellar...')
    try {
        const compilationOut = await execSync(`cmake . && cmake --build .`).toString()
        console.log(compilationOut)
    } catch (e) {
        process.exit(1)
    }
    console.log('Finished compiling')
    const executablePath = path.join(rootPath, 'exe')
    const e2ePath = path.join(rootPath, 'test', 'e2e')
    const e2eOutputPath = path.join(e2ePath, 'output')
    const stdFolder = path.join(rootPath, 'std')
    const outputPath = path.join(rootPath, 'output.o')
    const runnerPath = path.join(rootPath, 'runner.cpp')
    const mainPath = path.join(__dirname, 'main')
    const files = await fs.readdirSync(e2ePath)
    let numTestsPassed = 0
    for (const file of files) {
        const fileBaseName = path.basename(file, path.extname(file))
        if (path.extname(file) === '.stellar') {
            // todo: test timeout
            try {
                const stdout = (await execSync(`${executablePath} ${path.join(e2ePath, file)} ${stdFolder} >> ${logPath} && clang++ ${runnerPath} ${outputPath} -o ${mainPath} -static && ${mainPath}`)).toString()
                const content = await fs.readFileSync(path.join(e2eOutputPath, fileBaseName + '.output')).toString()
                if (stdout !== content) {
                    console.error(`Test failed: ${fileBaseName}`)
                    console.log('Output:')
                    console.log(stdout)
                    console.log('Expected:')
                    console.log(content)
                    console.log('-----------------------------------------------')
                } else {
                    numTestsPassed++
                    console.log(`Test passed: ${fileBaseName}`)
                }
            } catch (e) {
                console.error(`Test failed: ${fileBaseName} (compilation error)`)
            }
        }
    }
    console.log(`Passed ${numTestsPassed}/${files.length - 1} tests`)
    if (numTestsPassed !== files.length - 1) {
            process.exit(1)
    }
}

runTests()
