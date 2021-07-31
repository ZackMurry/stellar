const fs = require('fs')
const path = require('path')
const { execSync } = require('child_process')

const runTests = async () => {
    const logPath = path.join(__dirname, 'e2e.log')
    const rootPath = path.join(__dirname, '..')
    if (path.basename(rootPath) !== 'stellar') {
        console.error('Error: wrong working directory. Your working directory should be the root directory of the project (the stellar folder)')
        return
    }
    if (await fs.existsSync(logPath)) {
        await fs.rmSync(logPath)
    }
    console.log('Compiling stellar...')
    await execSync(`cmake --build ${rootPath}`)
    console.log('Finished compiling')
    const executablePath = path.join(rootPath, 'exe')
    const e2ePath = path.join(rootPath, 'test', 'e2e')
    const e2eOutputPath = path.join(e2ePath, 'output')
    const stdFolder = path.join(rootPath, 'std')
    const outputPath = path.join(__dirname, 'output.o')
    const runnerPath = path.join(rootPath, 'runner.cpp')
    const mainPath = path.join(__dirname, 'main')
    const files = await fs.readdirSync(e2ePath)
    let numTestsPassed = 0
    for (const file of files) {
        const fileBaseName = path.basename(file, path.extname(file))
        if (path.extname(file) === '.stellar') {
            const stdout = await execSync(`${executablePath} ${path.join(e2ePath, file)} ${stdFolder} >> ${logPath} && clang++ ${runnerPath} ${outputPath} -o main -static && ${mainPath}`).toString()
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
        }
    }
    console.log(`Passed ${numTestsPassed}/${files.length - 1} tests`)
}

runTests()
