find src -type f \
  -not -path "src/glad/*" \
  -not -path "src/glm/*" \
  -not -path "src/KHR/*" \
  -not -path "src/vendor/*" \
  -exec wc -l {} + | sort -n
