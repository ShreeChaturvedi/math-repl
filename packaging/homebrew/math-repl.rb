class MathRepl < Formula
  desc "Math REPL in C++23"
  homepage "https://github.com/ShreeChaturvedi/math-repl"
  url "https://github.com/ShreeChaturvedi/math-repl/archive/refs/tags/v0.1.0.tar.gz"
  sha256 "c307b44d5c52f0639418a790a0b8c0d195436a6c30fb91dec38854010a129974"
  license "MIT"

  depends_on "cmake" => :build

  def install
    system "cmake", "-S", ".", "-B", "build", "-DREPL_BUILD_TESTS=OFF", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    output = pipe_output("#{bin}/repl", "2 + 2\nexit\n")
    assert_match "4", output
  end
end
