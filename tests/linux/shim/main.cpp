// Test runner. Each test runs in a forked child so that an assert()/abort() inside the
// code under test is reported as a failure of that test instead of killing the run.
// Failures listed in KNOWN_FAILURES_FILE (one test name per line, '#' comments) are
// reported but don't fail the run; anything else failing does.
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <set>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include "CppUnitTest.h"

static std::set<std::string> LoadKnownFailures()
{
	std::set<std::string> names;
#ifdef KNOWN_FAILURES_FILE
	std::ifstream in(KNOWN_FAILURES_FILE);
	std::string line;
	while (std::getline(in, line))
	{
		const auto hash = line.find('#');
		if (hash != std::string::npos) line.erase(hash);
		while (!line.empty() && isspace((unsigned char)line.back())) line.pop_back();
		while (!line.empty() && isspace((unsigned char)line.front())) line.erase(line.begin());
		if (!line.empty()) names.insert(line);
	}
#endif
	return names;
}

// Runs one test in a child; returns an empty string on success, otherwise the failure text.
static std::string RunIsolated(const cut::Case& c)
{
	int fds[2];
	if (pipe(fds) != 0) return "pipe() failed";
	fflush(nullptr);
	const pid_t pid = fork();
	if (pid == 0)
	{
		close(fds[0]);
		std::string msg;
		try { c.fn(); _exit(0); }
		catch (const std::exception& e) { msg = e.what(); }
		catch (...) { msg = "unknown exception"; }
		if (write(fds[1], msg.data(), msg.size()) < 0) {}
		_exit(1);
	}
	close(fds[1]);
	std::string msg;
	char buf[512];
	ssize_t n;
	while ((n = read(fds[0], buf, sizeof buf)) > 0) msg.append(buf, (size_t)n);
	close(fds[0]);
	int status = 0;
	waitpid(pid, &status, 0);
	if (WIFSIGNALED(status)) return "crashed (signal " + std::to_string(WTERMSIG(status)) + ", e.g. failed assert)";
	if (WEXITSTATUS(status) != 0) return msg.empty() ? "failed" : msg;
	return "";
}

int main()
{
	setvbuf(stdout, nullptr, _IONBF, 0);
	const auto known = LoadKnownFailures();
	int failed = 0, knownFailed = 0;
	for (const auto& c : cut::Registry())
	{
		const std::string err = RunIsolated(c);
		if (err.empty())
		{
			std::printf("[ OK ] %s\n", c.name);
		}
		else if (known.count(c.name))
		{
			++knownFailed;
			std::printf("[KNOWN] %s: %s\n", c.name, err.c_str());
		}
		else
		{
			++failed;
			std::printf("[FAIL] %s: %s\n", c.name, err.c_str());
		}
	}
	std::printf("%d tests, %d failed, %d known failures\n", (int)cut::Registry().size(), failed, knownFailed);
	return failed ? 1 : 0;
}
