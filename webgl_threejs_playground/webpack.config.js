const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

module.exports = {
  mode: 'development',
  entry: './src/index.ts',
  devtool: 'inline-source-map',
  output: {
    filename: 'bundle.js',
    path: path.resolve(__dirname, 'dist'),
  },
  module: {
    rules: [
      {
        test: /\.ts$/,
        use: 'ts-loader',
        exclude: /node_modules/,
      },
      {
        test: /\.png$|\.jpg$/,
        use: 'file-loader',
        exclude: /node_modules/,
      },
      {
        test: /\.glsl$|\.vert$|\.frag$/,
        exclude: /node_modules/,
        use: [
          {
            loader: 'raw-loader'
          },
          {
            loader: 'glslify-loader'
          }
        ]
      }
    ]
  },
  plugins: [
    new HtmlWebpackPlugin({
      template: './public/index.html'
    })
  ],
  resolve: {
    extensions: ['.ts', '.js']
  },
  devServer: {
    port: 3000,
  }
}
